#ifndef COLOR_H
#define COLOR_H

#include <QColor>

struct Color {
   float l99;
   float a99;
   float b99;

   Color();
   Color(float l99, float a99, float b99);
   explicit Color(QRgb rgb);
   QRgb toQRgb() const;
   double magnitudeSquared() const;

   float c() const;
   float h() const;

   Color operator+(Color const & other) const;
   Color & operator+=(Color const & other);
   Color operator-(Color const & other) const;
   Color & operator-=(Color const & other);
   Color operator*(double a) const;
   Color & operator*=(double a);
   Color operator/(double a) const;
   Color & operator/=(double a);

private:
   static double cbrt(double x);
   double cube(double x) const;

};

#endif // COLORDIN99_H
