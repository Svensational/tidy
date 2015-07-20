#ifndef MEANSHIFTDECOMPOSER_H
#define MEANSHIFTDECOMPOSER_H

#include <QMultiHash>
#include "decomposer.h"

class QSpinBox;
class QDoubleSpinBox;
struct Pixel;
using Lattice = QMultiHash<QPair<int, int>, Pixel>;

class MeanShiftDecomposer : public Decomposer {

public:
   MeanShiftDecomposer();
   virtual SegmentList decompose(ImageColor const & image) const;
   virtual SegmentList decomposeBatch(ImageColor const & image, QString const & name) const;

protected:
   QDoubleSpinBox * sigmaPos;
   QDoubleSpinBox * sigmaCol;
   QSpinBox * minSize;
   QDoubleSpinBox * epsilonShift;
   QDoubleSpinBox * epsilonMerge;

   void populateSettingsLayout();
   ImageColor filter(ImageColor const & image) const;
   SegmentList labelRegions(ImageColor const & filtered,
                            ImageColor const & image) const;
};

struct FilterData {
   Pixel const & pixel;
   Lattice const & lattice;
   double sigmaPos;
   double sigmaCol;
   double epsSquared;
};

Pixel filterMT(FilterData const & data);

#endif // MEANSHIFTDECOMPOSER_H
