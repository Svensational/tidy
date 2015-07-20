#ifndef WATERSHEDDECOMPOSER_H
#define WATERSHEDDECOMPOSER_H

#include "decomposer.h"

class QSpinBox;
class QDoubleSpinBox;

class WaterShedDecomposer : public Decomposer {

public:
   WaterShedDecomposer();
   virtual SegmentList decompose(ImageColor const & image) const;
   virtual SegmentList decomposeBatch(ImageColor const & image, QString const &) const;

protected:
   QSpinBox * radiusGauss;
   QSpinBox * minSize;
   QDoubleSpinBox * epsilonMerge;

   void populateSettingsLayout();

   ImageGray gradientMagnitude(ImageColor const & image) const;
   SegmentList watershed(ImageGray const & gradient,
                         ImageColor const & image) const;
   ImageColor filterGauss(ImageColor const & image, int r) const;

   ImageColor filterGaussSinglePass(ImageColor const & image, int r) const;
   unsigned long long nCr(int n, int r) const;

   struct GradPixelRef {
      float gradientMagnitude;
      int index;
   };
   static bool lessThan(GradPixelRef const & a, GradPixelRef const & b);
};

#endif // WATERSHEDDECOMPOSER_H
