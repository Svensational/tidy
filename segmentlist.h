#ifndef SEGMENTLIST_H
#define SEGMENTLIST_H

#include <QList>
#include "image.forward.h"

class Segment;
class Position;

class SegmentList : public QList<Segment *> {

public:
   void deleteAndClear();
   void copyToImageAVG(ImageColor & image) const;

   void prepare();
   void calculateMeanColors();
   void calculateFeatureVariances();
   void resetAngles();

   int area() const;
   Position center() const;
   Position bottomCenter() const;
   Position topCenter() const;
   QRectF rect() const;
   int featX() const;
   int featY() const;

   void moveTo(Position const & newCenter);
   void translate(Position const & translation);

private:
   int mostSigni[2];

   void normalizeFeatures();
};

#endif // SEGMENTLIST_H
