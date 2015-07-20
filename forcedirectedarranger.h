#ifndef ARRANGER1_H
#define ARRANGER1_H

#include "arranger.h"

class QCheckBox;
class QComboBox;

class ForceDirectedArranger : public Arranger {

public:
   ForceDirectedArranger();
   virtual QGraphicsScene * arrange(SegmentList const & segments) const;
   virtual void arrangeBatch(SegmentList const & segments, QString const & name) const;

private:
   QComboBox * xAxisBox;
   QComboBox * yAxisBox;
   QCheckBox * rotationCB;

   void populateSettingsLayout();
   void refineLayoutSimple(SegmentList & segments) const;
   void refineLayoutWRotate(SegmentList & segments) const;
};

#endif // ARRANGER1_H
