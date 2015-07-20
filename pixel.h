#ifndef PIXEL_H
#define PIXEL_H

#include "color.h"

struct Position {
   float x;
   float y;

   Position();
   Position(float x, float y);
   double magnitude() const;
   double magnitudeSquared() const;
   Position rounded() const;
   Position normalized() const;

   Position operator+(Position const & other) const;
   Position & operator+=(Position const & other);
   Position operator-() const;
   Position operator-(Position const & other) const;
   Position & operator-=(Position const & other);
   Position operator*(double a) const;
   Position & operator*=(double a);
   Position operator*(Position const & other) const;
   Position & operator*=(Position const & other);
   Position operator/(double a) const;
   Position & operator/=(double a);
};

double dot(Position const & a, Position const & b);

struct Pixel {
   Position pos;
   Color col;

   Pixel();
   Pixel(Position position, Color color);
   double magnitudeSquared() const;

   Pixel operator+(Pixel const & other) const;
   Pixel & operator+=(Pixel const & other);
   Pixel operator-(Pixel const & other) const;
   Pixel & operator-=(Pixel const & other);
   Pixel operator*(double a) const;
   Pixel & operator*=(double a);
   Pixel operator/(double a) const;
   Pixel & operator/=(double a);
   bool operator<(Pixel const & other) const;
};

#endif // PIXEL_H
