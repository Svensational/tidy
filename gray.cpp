#include "gray.h"

Gray::Gray() :
   l(0.0f)
{
}

Gray::Gray(float l) :
   l(l)
{
}

Gray::Gray(double l) :
   l(l)
{
}

Gray::Gray(QRgb rgb) :
   l(qGray(rgb))
{
}

double Gray::magnitudeSquared() const {
   return l*l;
}

Gray Gray::operator+(Gray const & other) const {
   return Gray(l + other.l);
}

Gray & Gray::operator+=(Gray const & other) {
   l += other.l;
   return *this;
}

Gray Gray::operator-(Gray const & other) const {
   return Gray(l - other.l);
}

Gray & Gray::operator-=(Gray const & other) {
   l -= other.l;
   return *this;
}

Gray Gray::operator*(float a) const {
   return Gray(l*a);
}

Gray & Gray::operator*=(float a) {
   l *= a;
   return *this;
}

Gray Gray::operator/(float a) const {
   return Gray(l/a);
}

Gray & Gray::operator/=(float a) {
   l /= a;
   return *this;
}

QRgb Gray::toQRgb() const {
   int gray = qBound(0, qRound(l), 255);
   return qRgb(gray, gray, gray);
}
