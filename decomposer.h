#ifndef DECOMPOSER_H
#define DECOMPOSER_H

#include <QList>
#include "image.forward.h"

class QLayout;
class QFormLayout;
class Segment;
class SegmentList;

class Decomposer {

public:
   explicit Decomposer(QString const & name);
   virtual ~Decomposer();
   virtual SegmentList decompose(ImageColor const & image) const = 0;
   virtual SegmentList decomposeBatch(ImageColor const & image, QString const & name) const = 0;
   QString getName() const;
   QLayout * getSettingsLayout() const;

protected:
   QString name;

   QFormLayout * settingsLayout;

   void mergeSimiliarSegments(SegmentList & segments, double epsSquared = 1.0) const;
   void mergeSmallSegments(SegmentList & segments, int minSize = 10) const;
   void merge(QList<QPair<Segment *, Segment *>> & mergelist, SegmentList & segments) const;
};

#endif // DECOMPOSER_H
