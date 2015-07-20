#include "watersheddecomposer.h"
#include <algorithm>
#include <memory>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QSpinBox>
#include <QTime>
#include "image.h"
#include "pixel.h"
#include "segment.h"
#include "segmentlist.h"

WaterShedDecomposer::WaterShedDecomposer() :
   Decomposer("Watershed Decomposer")
{
   populateSettingsLayout();
}

SegmentList WaterShedDecomposer::decompose(ImageColor const & image) const {
   QTime time;
   time.start();

   // original image
   image.save("WS1_original.png");

   // filter image
   time.restart();
   ImageColor filtered = filterGauss(image, radiusGauss->value());
   qDebug("Image filtered in %g seconds", time.restart()/1000.0);
   filtered.save("WS2_filtered.png");

   // calculate gradient magnitude map
   time.restart();
   ImageGray gradientMap = gradientMagnitude(filtered);
   qDebug("Gradient magnitude map calculated in %g seconds", time.restart()/1000.0);
   gradientMap.save("WS3_gradient.png");

   // apply watershed transformation
   time.restart();
   SegmentList segments = watershed(gradientMap, image);
   qDebug("Watershed transformation applied in %g seconds", time.restart()/1000.0);
   qDebug("  Segments: %d", segments.size());
   ImageColor debugOut(image.width(), image.height());
   segments.copyToImageAVG(debugOut);
   debugOut.save("WS4_transformed.png");

   // merge similiar and small segments
   time.restart();
   int oldSegmentsSize;
   do {
      oldSegmentsSize = segments.size();
      mergeSimiliarSegments(segments, epsilonMerge->value()*epsilonMerge->value());
      mergeSmallSegments(segments, minSize->value());
   } while (segments.size() != oldSegmentsSize);
   qDebug("Segments merged in %g seconds", time.restart()/1000.0);
   qDebug("  Segments: %d", segments.size());
   segments.copyToImageAVG(debugOut);
   debugOut.save("WS5_merged.png");

   return segments;
}

SegmentList WaterShedDecomposer::decomposeBatch(ImageColor const & image, QString const &) const {
   return decompose(image);
}

ImageColor WaterShedDecomposer::filterGauss(ImageColor const & image, int r) const {
   return filterGaussSinglePass(filterGaussSinglePass(image, r), r);
}

ImageColor WaterShedDecomposer::filterGaussSinglePass(ImageColor const & image, int r) const {
   r = std::max(1, r);
   int n = (r<<1) + 1;

   // create kernel
   unsigned long long * kernel = new unsigned long long[n];
   for (int i=0; i<r; ++i) {
      kernel[i] = kernel[n-i-1] = nCr(n-1, i);
   }
   kernel[r] = nCr(n-1, r);

   ImageColor filtered(image.height(), image.width());
   for (int y=0; y<image.height(); ++y) {
      for (int x=0; x<image.width(); ++x) {
         Color color;
         unsigned long long weights = 0;
         for (int i=0; i<n; ++i) {
            if (x-r+i >= 0 && x-r+i < image.width()) {
               color += image.at(x-r+i, y) * kernel[i];
               weights += kernel[i];
            }
         }
         filtered.at(y, x) = color / double(weights);
      }
   }

   delete[] kernel;

   return filtered;
}

ImageGray WaterShedDecomposer::gradientMagnitude(ImageColor const & image) const {
   ImageGray gmImage(image.width(), image.height());

   // Frobenius norm on Jacobian matrix
   for (int y=0; y<image.height(); ++y) {
      for (int x=0; x<image.width(); ++x) {
         gmImage.at(x, y) = Gray(sqrt((image.at(std::min(image.width()-1, x+1), y)-
                                  image.at(std::max(0, x-1), y)).magnitudeSquared() +
                                 (image.at(x, std::min(image.height()-1, y+1))-
                                  image.at(x, std::max(0, y-1))).magnitudeSquared()));
      }
   }

   // scaling just for debug
   float max = 0.0;
   for (int i=0; i<gmImage.area(); ++i) {
      max = std::max(max, gmImage.at(i).l);
   }
   for (int i=0; i<gmImage.area(); ++i) {
      gmImage.at(i).l *= 255.0/max;
   }

   return gmImage;
}

bool WaterShedDecomposer::lessThan(GradPixelRef const & a, GradPixelRef const & b) {
   return a.gradientMagnitude < b.gradientMagnitude;
}

unsigned long long WaterShedDecomposer::nCr(int n, int r) const {
   if (r<<1 > n) {
      return nCr(n, n-r);
   }
   else {
      unsigned long long result = 1;
      for (int i=1; i<=r; ++i) {
         result *= n - r + i;
         result /= i;
      }
      return result;
   }
}

void WaterShedDecomposer::populateSettingsLayout() {
   radiusGauss = new QSpinBox();
   radiusGauss->setRange(1, 31);
   radiusGauss->setValue(31);
   radiusGauss->setToolTip(QObject::tr("Kernel radius of the Gaussian blur filter"));
   settingsLayout->addRow(QObject::tr("Gauß kernel radius"), radiusGauss);

   minSize = new QSpinBox();
   minSize->setRange(1, 1000);
   minSize->setValue(50);
   minSize->setToolTip(QObject::tr("The minimal allowed segment size (smaller segments will be merged)"));
   settingsLayout->addRow(QObject::tr("Minimum size"), minSize);

   epsilonMerge = new QDoubleSpinBox();
   epsilonMerge->setRange(0.5, 50.0);
   epsilonMerge->setValue(3.0);
   epsilonMerge->setSingleStep(0.1);
   epsilonMerge->setToolTip(QObject::tr("The minimum color space distance (nearer segments will be merged)"));
   settingsLayout->addRow(QChar(949)+QObject::tr(" merge"), epsilonMerge);
}

SegmentList WaterShedDecomposer::watershed(ImageGray const & gradient,
                                           ImageColor const & image) const {
   SegmentList segments;
   std::unique_ptr<int[]> labels(new int[image.area()]);
   for (int i=0; i<image.area(); ++i) labels[i] = -1;

   int offsets[]{-image.width(), -1, 1, image.width()};

   QList<GradPixelRef> queue;
   for (int i=0; i<gradient.area(); ++i) {
      queue << GradPixelRef{gradient.at(i).l, i};
   }
   std::sort(queue.begin(), queue.end(), lessThan);

   int label;
   int lastLabel = -1;
   int i, j;
   QList<int> neighLbls;
   Pixel * pixel;
   Segment * segment;
   foreach (GradPixelRef const & gradPix, queue) {
      i = gradPix.index;
      pixel = new Pixel(Position(i%image.width(), i/image.width()), image.at(i));

      // gather neighbours
      neighLbls.clear();
      for (int o=0; o<4; ++o) {
         j = i + offsets[o];
         if (image.areNeighbours(i, j) && labels[j] > -1 && !neighLbls.contains(labels[j])) {
            neighLbls << labels[j];
         }
      }

      // treat pixel according to neighbour count
      switch (neighLbls.size()) {
      case 0: // new marker
         labels[i] = ++lastLabel;
         segment = new Segment();
         segment->addPixel(pixel);
         segments << segment;
         break;
      case 1: // add to basin
         label = neighLbls.first();
         labels[i] = label;
         segments.at(label)->addPixel(pixel);
         break;
      default: // new watershed
         // add pixel the segment of the nearest neighbour in color
         double distMin = std::numeric_limits<double>::max();
         double dist;
         int jMin = 0;
         for (int o=0; o<4; ++o) {
            j = i + offsets[o];
            if (image.areNeighbours(i, j) && labels[j] > -1) {
               dist = (image.at(i)-image.at(j)).magnitudeSquared();
               if (dist < distMin) {
                  distMin = dist;
                  jMin = j;
               }
            }
         }
         labels[i] = labels[jMin];
         segments.at(labels[jMin])->addPixel(pixel);

         // beneighbour the segments
         for (int k=0; k<neighLbls.count()-1; ++k) {
            for (int l=k+1; l<neighLbls.count(); ++l) {
               segments.at(neighLbls.at(k))->addNeighbour(segments.at(neighLbls.at(l)));
               segments.at(neighLbls.at(l))->addNeighbour(segments.at(neighLbls.at(k)));
            }
         }
         break;
      }
   }

   segments.calculateMeanColors();
   return segments;
}
