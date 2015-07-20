#include "pixel.h"

Position::Position() :
   x(0.0f), y(0.0f)
{
}

Position::Position(float x, float y) :
   x(x), y(y)
{
}

double Position::magnitude() const {
   return sqrt(magnitudeSquared());
}

double Position::magnitudeSquared() const {
   return x*x + y*y;
}

Position Position::normalized() const {
   return *this / sqrt(magnitudeSquared());
}

Position Position::operator+(Position const & other) const {
   return Position(x+other.x,
                   y+other.y);
}

Position & Position::operator+=(Position const & other) {
   x += other.x;
   y += other.y;
   return *this;
}

Position Position::operator-() const {
   return Position(-x,
                   -y);
}

Position Position::operator-(Position const & other) const {
   return Position(x-other.x,
                   y-other.y);
}

Position & Position::operator-=(Position const & other) {
   x -= other.x;
   y -= other.y;
   return *this;
}

Position Position::operator*(double a) const {
   return Position(x*a, y*a);
}

Position & Position::operator*=(double a) {
   x *= a;
   y *= a;
   return *this;
}

Position Position::operator*(Position const & other) const {
   return Position(x*other.x, y*other.y);
}

Position & Position::operator*=(Position const & other) {
   x *= other.x;
   y *= other.y;
   return *this;
}

Position Position::operator/(double a) const {
   return Position(x/a, y/a);
}

Position & Position::operator/=(double a) {
   x /= a;
   y /= a;
   return *this;
}

Position Position::rounded() const {
   return Position(qRound(x), qRound(y));
}

////////////////////////////////////////////////////////////////////////////////

double dot(Position const & a, Position const & b) {
   return a.x*b.x + a.y*b.y;
}

////////////////////////////////////////////////////////////////////////////////

Pixel::Pixel() :
   pos(Position()), col(Color())
{
}

Pixel::Pixel(Position position, Color color) :
   pos(position), col(color)
{
}

double Pixel::magnitudeSquared() const {
   return pos.magnitudeSquared() + col.magnitudeSquared();
}

Pixel Pixel::operator+(Pixel const & other) const {
   return Pixel(pos+other.pos,
                col+other.col);
}

Pixel & Pixel::operator+=(Pixel const & other) {
   pos += other.pos;
   col += other.col;
   return *this;
}

Pixel Pixel::operator-(Pixel const & other) const {
   return Pixel(pos-other.pos,
                col-other.col);
}

Pixel & Pixel::operator-=(Pixel const & other) {
   pos -= other.pos;
   col -= other.col;
   return *this;
}

Pixel Pixel::operator*(double a) const {
   return Pixel(pos*a, col*a);
}

Pixel & Pixel::operator*=(double a) {
   pos *= a;
   col *= a;
   return *this;
}

Pixel Pixel::operator/(double a) const {
   return Pixel(pos/a, col/a);
}

Pixel & Pixel::operator/=(double a) {
   pos /= a;
   col /= a;
   return *this;
}

bool Pixel::operator<(Pixel const & other) const {
   if (qFuzzyCompare(pos.y, other.pos.y)) {
      return pos.x < other.pos.x;
   }
   else {
      return pos.y < other.pos.y;
   }
}
