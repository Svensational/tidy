#ifndef CLUSTEREDARRANGER_H
#define CLUSTEREDARRANGER_H

#include "arranger.h"

class QComboBox;

class ClusteredArranger : public Arranger {

public:
   ClusteredArranger();
   virtual QGraphicsScene * arrange(SegmentList const & segments) const;
   virtual void arrangeBatch(SegmentList const & segments, QString const & name) const;

private:
   QComboBox * xAxisBox;
   QComboBox * yAxisBox;
   QComboBox * clusterBox;

   QList<SegmentList> meanShift(SegmentList const & segments) const;
   void refineLayoutCircles(SegmentList & segments) const;
   void refineLayoutPiles(SegmentList & segments) const;
   void refineLayoutBySize(QList<SegmentList> & clusters) const;
   void refineLayoutByPlace(QList<SegmentList> & clusters) const;

   void populateSettingsLayout();
   double minDist(SegmentList const & listA, SegmentList const & listB) const;
   static bool biggerThan(SegmentList const & listA, SegmentList const & listB);
   static bool biggerAreaThan(SegmentList const & listA, SegmentList const & listB);
};

#endif // CLUSTEREDARRANGER_H
