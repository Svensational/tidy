#include "meanshiftdecomposer.h"
#include <memory>
#include <QtConcurrent>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QProgressDialog>
#include <QSpinBox>
#include <QTime>
#include "pixel.h"
#include "image.h"
#include "segment.h"
#include "segmentlist.h"

MeanShiftDecomposer::MeanShiftDecomposer() :
   Decomposer("Mean Shift Decomposer")
{
   populateSettingsLayout();
}

SegmentList MeanShiftDecomposer::decompose(ImageColor const & image) const {
   QTime time;
   time.start();

   // original image
   image.save("MS1_original.png");

   // filter image
   time.restart();
   ImageColor imageFiltered = filter(image);
   qDebug("Data filtered in %g seconds", time.restart()/1000.0);
   imageFiltered.save("MS2_filtered.png");

   // label regions
   time.restart();
   SegmentList segments = labelRegions(imageFiltered, image);
   qDebug("Regions labeled in %g seconds", time.restart()/1000.0);
   qDebug("  Segments: %d", segments.size());
   segments.copyToImageAVG(imageFiltered);
   imageFiltered.save("MS3_labeled.png");

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
   segments.copyToImageAVG(imageFiltered);
   imageFiltered.save("MS4_merged.png");

   return segments;
}

SegmentList MeanShiftDecomposer::decomposeBatch(ImageColor const & image, QString const & name) const {
   QDir dir;
   dir.mkpath(name);

   QFile file(name + "/output.txt");
   file.open(QFile::WriteOnly | QFile::Truncate);
   QTextStream out(&file);
   out << "Mean Shift Decomposer" << endl;
   out << "   Sigma pos: " << sigmaPos->value() << endl;
   out << "   Sigma col: " << sigmaCol->value() << endl;
   out << "   Minimum Size: " << minSize->value() << endl;
   out << "   eps shift: " << epsilonShift->value() << endl;
   out << "   eps merge: " << epsilonMerge->value() << endl;
   out << endl;

   QTime time;
   time.start();

   // original image
   image.save(name + "/MS1_original.png");

   // filter image
   time.restart();
   ImageColor imageFiltered = filter(image);
   out << "Data filtered in " << time.restart()/1000.0 << " seconds" << endl;
   imageFiltered.save(name + "/MS2_filtered.png");

   // label regions
   time.restart();
   SegmentList segments = labelRegions(imageFiltered, image);
   out << "Regions labeled in " << time.restart()/1000.0 << " seconds" << endl;
   out << "   Segments: " << segments.size() << endl;
   segments.copyToImageAVG(imageFiltered);
   imageFiltered.save(name + "/MS3_labeled.png");

   // merge similiar and small segments
   time.restart();
   int oldSegmentsSize;
   do {
      oldSegmentsSize = segments.size();
      mergeSimiliarSegments(segments, epsilonMerge->value()*epsilonMerge->value());
      mergeSmallSegments(segments, minSize->value());
   } while (segments.size() != oldSegmentsSize);
   out << "Segments merged in " << time.restart()/1000.0 << " seconds" << endl;
   out << "   Segments: " << segments.size() << endl;
   segments.copyToImageAVG(imageFiltered);
   imageFiltered.save(name + "/MS4_merged.png");

   return segments;
}

ImageColor MeanShiftDecomposer::filter(ImageColor const & image) const {
   // create lattice
   Lattice lattice;
   for (int y=0; y<image.height(); ++y) {
      for (int x=0; x<image.width(); ++x) {
         lattice.insert(QPair<int, int>(qRound(x / sigmaPos->value()),
                                        qRound(y / sigmaPos->value())),
                        Pixel(Position(x, y) / sigmaPos->value(),
                              image.at(x, y) / sigmaCol->value()));
      }
   }

   // create data for mapped filter
   QList<FilterData> filterData;
   foreach (Pixel const & pixel, lattice) {
      filterData << FilterData{pixel, lattice,
                               sigmaPos->value(), sigmaCol->value(),
                               epsilonShift->value()*epsilonShift->value()};
   }

   // filter
   QProgressDialog progress("Applying mean shift filter...", "Abort", 0, image.area());
   progress.setMinimumDuration(0);
   progress.setModal(true);
   QFuture<Pixel> future = QtConcurrent::mapped(filterData, filterMT);
   while (future.isRunning()) {
      progress.setValue(future.progressValue());
      if(progress.wasCanceled()) {
         future.cancel();
      }
      qApp->processEvents();
   }
   QList<Pixel> dataFiltered = future.results();
   //QList<Pixel> dataFiltered = QtConcurrent::blockingMapped(filterData, filterMT);

   // store filtered data in a Luv image
   ImageColor imageFiltered(image.width(), image.height());
   foreach (Pixel const & pixel, dataFiltered) {
      imageFiltered.at(qRound(pixel.pos.x), qRound(pixel.pos.y)) = pixel.col;
   }

   return imageFiltered;
}

SegmentList MeanShiftDecomposer::labelRegions(ImageColor const & filtered,
                                              ImageColor const & image) const {
   SegmentList segments;
   std::unique_ptr<int[]> labels(new int[filtered.area()]);
   for (int i=0; i<filtered.area(); ++i) labels[i] = -1;

   int offsets[]{-filtered.width(), -1, 1, filtered.width(),
                 -filtered.width()-1, -filtered.width()+1,
                 filtered.width()-1, filtered.width()+1};

   double const epsilonMergeSquared = epsilonMerge->value() * epsilonMerge->value();
   int lastLabel = -1;
   QQueue<int> queue;
   Color color;
   int colorCount;
   QList<Pixel *> pixels;
   int j, k;
   for (int i=0; i<filtered.area(); ++i) {
      if (labels[i] < 0) {
         // create new label and start region growing
         labels[i] = ++lastLabel;
         queue.enqueue(i);
         color = Color();
         colorCount = 0;
         pixels.clear();
         while (!queue.isEmpty()) {
            j = queue.dequeue();
            color += filtered.at(j);
            ++colorCount;
            pixels << new Pixel(Position(j%image.width(), j/image.width()),
                                image.at(j));
            // check all neighbours
            for (int o=0; o<4; ++o) {
               k = j+offsets[o];
               if (filtered.areNeighbours(j, k) &&
                   labels[k] < 0 &&
                   (filtered.at(k)-filtered.at(j)).magnitudeSquared() < epsilonMergeSquared) {
                  labels[k] = labels[i];
                  queue.enqueue(k);
               }
            }
         }
         color /= double(colorCount);
         segments << new Segment(color, pixels);
      }
   }

   // find neighbours
   for (int i=0; i<filtered.area(); ++i) {
      j = i+1;
      if (j%filtered.width()>0 && labels[j]!=labels[i]) {
         segments[labels[i]]->addNeighbour(segments[labels[j]]);
         segments[labels[j]]->addNeighbour(segments[labels[i]]);
      }
      j = i+filtered.width();
      if (j<filtered.area() && labels[j]!=labels[i]) {
         segments[labels[i]]->addNeighbour(segments[labels[j]]);
         segments[labels[j]]->addNeighbour(segments[labels[i]]);
      }
   }

   return segments;
}

void MeanShiftDecomposer::populateSettingsLayout() {
   sigmaPos = new QDoubleSpinBox();
   sigmaPos->setRange(1.0, 100.0);
   sigmaPos->setValue(16.0);
   sigmaPos->setToolTip(QObject::tr("The radius of the Mean Shift kernel in the spatial dimension"));
   settingsLayout->addRow(QChar(963)+QObject::tr(" position"), sigmaPos);

   sigmaCol = new QDoubleSpinBox();
   sigmaCol->setRange(1.0, 100.0);
   sigmaCol->setValue(8.0);
   sigmaCol->setToolTip(QObject::tr("The radius of the Mean Shift kernel in the color dimension"));
   settingsLayout->addRow(QChar(963)+QObject::tr(" color"), sigmaCol);

   minSize = new QSpinBox();
   minSize->setRange(1, 10000);
   minSize->setValue(50);
   minSize->setToolTip(QObject::tr("The minimal allowed segment size (smaller segments will be merged)"));
   settingsLayout->addRow(QObject::tr("Minimum size"), minSize);

   epsilonShift = new QDoubleSpinBox();
   epsilonShift->setDecimals(3);
   epsilonShift->setRange(0.001, 1.0);
   epsilonShift->setValue(0.03);
   epsilonShift->setSingleStep(0.01);
   epsilonShift->setToolTip(QObject::tr("The minimum threshold for the Mean Shift step width"));
   settingsLayout->addRow(QChar(949)+QObject::tr(" shift"), epsilonShift);

   epsilonMerge = new QDoubleSpinBox();
   epsilonMerge->setRange(0.5, 50.0);
   epsilonMerge->setValue(1.0);
   epsilonMerge->setSingleStep(0.1);
   epsilonMerge->setToolTip(QObject::tr("The minimum color space distance (nearer segments will be merged)"));
   settingsLayout->addRow(QChar(949)+QObject::tr(" merge"), epsilonMerge);
}

////////////////////////////////////////////////////////////////////////////////

Pixel filterMT(FilterData const & data) {
   Pixel center;
   Pixel nextCenter = data.pixel;
   double const weight = 1.0;
   double sumOfWeights;
   QPair<int, int> key;
   Lattice::const_iterator it;

   do {
      center = nextCenter;
      nextCenter = Pixel();
      sumOfWeights = 0.0;
      for (int dy=-1; dy<=1; ++dy) {
         for (int dx=-1; dx<=1; ++dx) {
            key = QPair<int, int>(qRound(center.pos.x)+dx,
                                  qRound(center.pos.y)+dy);
            it = data.lattice.find(key);
            while (it!=data.lattice.end() && it.key()==key) {
               if ((it.value()-center).magnitudeSquared() <= 1.0) {
                  nextCenter += it.value() * weight;
                  sumOfWeights += weight;
               }
               ++it;
            }
         }
      }
      nextCenter /= sumOfWeights;
   } while ((nextCenter - center).magnitudeSquared() > data.epsSquared);
   return Pixel(data.pixel.pos * data.sigmaPos, nextCenter.col * data.sigmaCol);
}
