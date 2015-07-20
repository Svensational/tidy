#include "color.h"

Color::Color() :
   l99(0.0f), a99(0.0f), b99(0.0f)
{
}

Color::Color(float l99, float a99, float b99) :
   l99(l99), a99(a99), b99(b99)
{
}

Color::Color(QRgb rgb) :
   l99(0.0f), a99(0.0f), b99(0.0f)
{
   if (qRed(rgb) + qGreen(rgb) + qBlue(rgb) > 0) {
      // convert RBG to XYZ
      double const x = 0.4124564*qRed(rgb) + 0.3575761*qGreen(rgb) + 0.1804375*qBlue(rgb);
      double const y = 0.2126729*qRed(rgb) + 0.7151522*qGreen(rgb) + 0.0721750*qBlue(rgb);
      double const z = 0.0193339*qRed(rgb) + 0.1191920*qGreen(rgb) + 0.9503041*qBlue(rgb);

      // convert XYZ to L*a*b*
      double const lStar = 116.0 * cbrt(y/255.0) - 16.0;
      double const aStar = 500.0 * (cbrt(x/242.25) - cbrt(y/255.0));
      double const bStar = 200.0 * (cbrt(y/255.0) - cbrt(z/277.95));

      // convert L*a*b* to DIN99
      double const e = aStar*0.96126169593832 + bStar*0.275637355817;
      double const f = 0.7 * (-aStar*0.275637355817 + bStar*0.96126169593832);
      double const g = sqrt(e*e + f*f);
      double const k = log(1.0 + 0.045 * g) / 0.045;
      l99 = 105.51 * log(1.0 + 0.0158 * lStar);
      a99 = k * e / g;
      b99 = k * f / g;
   }
}

double Color::cube(double x) const {
   if (x > 0.20689655172414) {
      return x*x*x;
   }
   else {
      return 0.12841854934602 * (x - 0.13793103448276);
   }
}

double Color::cbrt(double x) {
   if (x > 0.0088564516790356) {
      return pow(x, 0.33333333333333);
   }
   else {
      return 7.787037037037037 * x + 0.13793103448276;
   }
}

double Color::magnitudeSquared() const {
   return l99*l99 + a99*a99 + b99*b99;
}

Color Color::operator+(Color const & other) const {
   return Color(l99+other.l99,
                     a99+other.a99,
                     b99+other.b99);
}

Color & Color::operator+=(Color const & other) {
   l99 += other.l99;
   a99 += other.a99;
   b99 += other.b99;
   return *this;
}

Color Color::operator-(Color const & other) const {
   return Color(l99-other.l99,
                     a99-other.a99,
                     b99-other.b99);
}

Color & Color::operator-=(Color const & other) {
   l99 -= other.l99;
   a99 -= other.a99;
   b99 -= other.b99;
   return *this;
}

Color Color::operator*(double a) const {
   return Color(l99*a, a99*a, b99*a);
}

Color & Color::operator*=(double a) {
   l99 *= a;
   a99 *= a;
   b99 *= a;
   return *this;
}

Color Color::operator/(double a) const {
   return Color(l99/a, a99/a, b99/a);
}

Color & Color::operator/=(double a) {
   l99 /= a;
   a99 /= a;
   b99 /= a;
   return *this;
}

QRgb Color::toQRgb() const {
   if(qFuzzyIsNull(l99)) {
      return qRgb(0, 0, 0);
   }

   // convert DIN99 to L*a*b*
   double const h99ef = atan2(b99, a99);
   double const c99 = sqrt(a99*a99 + b99*b99);
   double const g = (exp(0.045*c99)-1.0)/0.045;
   double const e = g * cos(h99ef);
   double const f = g * sin(h99ef);
   double const l = (exp(l99/105.51) - 1.0) / 0.0158;
   double const a = e * 0.96126169593832 - (f/0.7) * 0.275637355817;
   double const b = e * 0.275637355817 + (f/0.7) * 0.96126169593832;

   // convert L*a*b* to XYZ
   double const x = 242.25 * cube(0.0086206896551724*(l+16.0)+0.002*a);
   double const y = 255.0 * cube(0.0086206896551724*(l+16.0));
   double const z = 277.95 * cube(0.0086206896551724*(l+16.0)-0.005*b);

   // convert XYZ to RGB
   return qRgb(qBound(0, qRound( 3.2404548*x - 1.5371389*y - 0.4985315*z), 255),
               qBound(0, qRound(-0.9692664*x + 1.8760109*y + 0.0415561*z), 255),
               qBound(0, qRound( 0.0556434*x - 0.2040259*y + 1.0572252*z), 255));
}

float Color::c() const {
   return sqrt(a99*a99 + b99*b99);
}

float Color::h() const {
   return atan2(b99, a99);
}
