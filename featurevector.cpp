#include "featurevector.h"
#include <cmath>

FeatureVector::FeatureVector() {
   for (auto it=begin(); it!=end(); ++it) {
      *it = 0.0;
   }
}

FeatureVector FeatureVector::operator+(FeatureVector const & other) const {
   FeatureVector sum;
   for (int i=0; i<10; ++i) {
      sum[i] = operator[](i) + other[i];
   }
   return sum;
}

FeatureVector & FeatureVector::operator+=(FeatureVector const & other) {
   for (int i=0; i<10; ++i) {
      operator[](i) += other[i];
   }
   return *this;
}

FeatureVector FeatureVector::operator-(FeatureVector const & other) const {
   FeatureVector diff;
   for (int i=0; i<10; ++i) {
      diff[i] = operator[](i) - other[i];
   }
   return diff;
}

FeatureVector & FeatureVector::operator-=(FeatureVector const & other) {
   for (int i=0; i<10; ++i) {
      operator[](i) -= other[i];
   }
   return *this;
}

FeatureVector FeatureVector::operator*(double a) const {
   FeatureVector prod;
   for (int i=0; i<10; ++i) {
      prod[i] = operator[](i) * a;
   }
   return prod;
}

FeatureVector FeatureVector::operator*(FeatureVector const & other) const {
   FeatureVector prod;
   for (int i=0; i<10; ++i) {
      prod[i] = operator[](i) * other[i];
   }
   return prod;
}

FeatureVector & FeatureVector::operator*=(double a) {
   for (auto it=begin(); it!=end(); ++it) {
      *it *= a;
   }
   return *this;
}

FeatureVector & FeatureVector::operator*=(FeatureVector const & other) {
   for (int i=0; i<10; ++i) {
      operator[](i) *= other[i];
   }
   return *this;
}

FeatureVector FeatureVector::operator/(double a) const {
   FeatureVector quot;
   for (int i=0; i<10; ++i) {
      quot[i] = operator[](i) / a;
   }
   return quot;
}

FeatureVector FeatureVector::operator/(FeatureVector const & other) const {
   FeatureVector quot;
   for (int i=0; i<10; ++i) {
      quot[i] = operator[](i) / other[i];
   }
   return quot;
}

FeatureVector & FeatureVector::operator/=(double a) {
   for (auto it=begin(); it!=end(); ++it) {
      *it /= a;
   }
   return *this;
}

FeatureVector & FeatureVector::operator/=(FeatureVector const & other) {
   for (int i=0; i<10; ++i) {
      operator[](i) /= other[i];
   }
   return *this;
}

FeatureVector FeatureVector::compwiseMin(FeatureVector const & other) const {
   FeatureVector min;
   for (int i=0; i<10; ++i) {
      min[i] = std::min(operator[](i), other[i]);
   }
   return min;
}

void FeatureVector::setCompwiseMin(FeatureVector const & other) {
   *this = compwiseMin(other);
}

FeatureVector FeatureVector::compwiseMax(FeatureVector const & other) const {
   FeatureVector max;
   for (int i=0; i<10; ++i) {
      max[i] = std::max(operator[](i), other[i]);
   }
   return max;
}

void FeatureVector::setCompwiseMax(FeatureVector const & other) {
   *this = compwiseMax(other);
}

double FeatureVector::magnitude() const {
   return sqrt(magnitudeSquared());
}

double FeatureVector::magnitudeSquared() const {
   double magSqared = 0.0;
   for (auto it=begin(); it!=end(); ++it) {
      magSqared += *it * *it;
   }
   return magSqared;
}

QString FeatureVector::toString(unsigned short id) {
   switch (id) {
   case SIZE:
      return QString("Size");
   case SPATIALSD:
      return QString("Spatial SD");
   case COMPACTNESS:
      return QString("Compactness");
   case LIGHTNESS:
      return QString("Lightness");
   case GREENRED:
      return QString("Green-Red");
   case YELLOWBLUE:
      return QString("Yellow-Blue");
   case CROMA:
      return QString("Croma");
   case HUE:
      return QString("Hue");
   case COLORSD:
      return QString("Color SD");
   case ANGLE:
      return QString("Angle");
   default:
      return QString("Unknown");
   }
}
