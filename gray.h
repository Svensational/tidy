#ifndef COLORGRAY_H
#define COLORGRAY_H

#include <QColor>

struct Gray {
   float l;

   Gray();
   explicit Gray(float l);
   explicit Gray(double l);
   explicit Gray(QRgb rgb);
   QRgb toQRgb() const;
   double magnitudeSquared() const;

   Gray operator+(Gray const & other) const;
   Gray & operator+=(Gray const & other);
   Gray operator-(Gray const & other) const;
   Gray & operator-=(Gray const & other);
   Gray operator*(float a) const;
   Gray & operator*=(float a);
   Gray operator/(float a) const;
   Gray & operator/=(float a);
};

#endif // COLORGRAY_H
