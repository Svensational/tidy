#include "segmentlist.h"
#include <QGraphicsScene>
#include "image.h"
#include "segment.h"
#include <QDebug>

int SegmentList::area() const {
   int area = 0;

   foreach (Segment const * const segment, *this) {
      area += segment->area();
   }

   return area;
}

Position SegmentList::bottomCenter() const {
   Position pos(0.0, std::numeric_limits<double>::max());
   int area = 0;

   foreach (Segment const * const segment, *this) {
      pos.x += segment->position().x * segment->area();
      area += segment->area();

      pos.y = std::min(pos.y, segment->position().y);
   }
   pos.x /= double(area);

   return pos;
}

void SegmentList::calculateFeatureVariances() {
   // calculate average features
   FeatureVector average;
   foreach (Segment const * const segment, *this) {
      average += segment->features();
   }
   average /= size();

   // calculate variance of each feature
   FeatureVector var;
   foreach (Segment const * const segment, *this) {
      var += (segment->features() - average) * (segment->features() - average);
   }
   var /= size();

   // find the two most significant features
   if (var[0] > var[1]) {
      mostSigni[0] = 0;
      mostSigni[1] = 1;
   }
   else {
      mostSigni[0] = 1;
      mostSigni[1] = 0;
   }
   // last two features get skipped because of their circular nature
   for (int i=2; i<8; ++i) {
      if (var[i] > var[mostSigni[0]]) {
         mostSigni[1] = mostSigni[0];
         mostSigni[0] = i;
      }
      else if (var[i] > var[mostSigni[1]]) {
         mostSigni[1] = i;
      }
   }

   // debug output
   /*for (int i=0; i<10; ++i) {
      qDebug("var %f", var[i]);
   }*/
   qDebug() << "x =" << FeatureVector::toString(mostSigni[0]) << " y =" << FeatureVector::toString(mostSigni[1]);
}

void SegmentList::calculateMeanColors() {
   foreach (Segment * const segment, *this) {
      segment->calculateColor();
   }
}

Position SegmentList::center() const {
   Position pos;
   int area = 0;

   foreach (Segment const * const segment, *this) {
      pos += segment->position() * segment->area();
      area += segment->area();
   }
   pos /= double(area);

   return pos;
}

void SegmentList::copyToImageAVG(ImageColor & image) const {
   foreach (Segment * const segment, *this) {
      segment->copyToImage(image, Position(), true);
   }
}

void SegmentList::deleteAndClear() {
   for (int i=0; i<size(); ++i) {
      delete operator[](i);
   }
   clear();
}

int SegmentList::featX() const {
   return mostSigni[0];
}

int SegmentList::featY() const {
   return mostSigni[1];
}

void SegmentList::moveTo(Position const & newCenter) {
   Position translation = newCenter - topCenter();

   foreach (Segment * const segment, *this) {
      segment->translate(translation);
   }
}

void SegmentList::normalizeFeatures() {
   if (isEmpty()) return;
   FeatureVector min = at(0)->features();
   FeatureVector max = at(0)->features();

   foreach (Segment const * const segment, *this) {
      min.setCompwiseMin(segment->features());
      max.setCompwiseMax(segment->features());
   }
   max = max - min;

   foreach (Segment * const segment, *this) {
      segment->features() = (segment->features() - min) / max;
   }
}

void SegmentList::prepare() {
   foreach (Segment * const segment, *this) {
      segment->relativizePosition();
      segment->calculateSpatialFeatures();
      segment->calculateColorFeatures();
      segment->calculateContour();
   }
   normalizeFeatures();
   calculateFeatureVariances();
}

Position SegmentList::topCenter() const {
   Position pos(0.0, std::numeric_limits<double>::min());
   int area = 0;

   foreach (Segment const * const segment, *this) {
      pos.x += segment->position().x * segment->area();
      area += segment->area();

      pos.y = std::max(pos.y, segment->position().y);
   }
   pos.x /= double(area);

   return pos;
}

void SegmentList::translate(Position const & translation) {
   foreach (Segment * const segment, *this) {
      segment->translate(translation);
   }
}

QRectF SegmentList::rect() const {
   QGraphicsScene scene;
   foreach(Segment const * const segment, *this) {
      scene.addItem(segment->toQGraphicsItem());
      // without the following line QPainter tends to crash
      scene.width();
   }
   return scene.sceneRect();
}

void SegmentList::resetAngles() {
   foreach(Segment * const segment, *this) {
      segment->resetAngle();
   }
}
