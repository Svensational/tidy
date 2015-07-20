#ifndef FEATUREVECTOR_H
#define FEATUREVECTOR_H

#include<array>
#include <QString>

enum Feature {SIZE, SPATIALSD, COMPACTNESS, LIGHTNESS, GREENRED, YELLOWBLUE, CROMA, COLORSD, HUE, ANGLE};

class FeatureVector : public std::array<double, 10> {

public:
   FeatureVector();

   FeatureVector operator+(FeatureVector const & other) const;
   FeatureVector & operator+=(FeatureVector const & other);

   FeatureVector operator-(FeatureVector const & other) const;
   FeatureVector & operator-=(FeatureVector const & other);

   FeatureVector operator*(double a) const;
   FeatureVector operator*(FeatureVector const & other) const;
   FeatureVector & operator*=(double a);
   FeatureVector & operator*=(FeatureVector const & other);

   FeatureVector operator/(double a) const;
   FeatureVector operator/(FeatureVector const & other) const;
   FeatureVector & operator/=(double a);
   FeatureVector & operator/=(FeatureVector const & other);

   FeatureVector compwiseMin(FeatureVector const & other) const;
   void setCompwiseMin(FeatureVector const & other);
   FeatureVector compwiseMax(FeatureVector const & other) const;
   void setCompwiseMax(FeatureVector const & other);

   double magnitude() const;
   double magnitudeSquared() const;

   static QString toString(unsigned short id);
};

#endif // FEATUREVECTOR_H
