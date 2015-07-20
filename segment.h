#ifndef SEGMENT_H
#define SEGMENT_H

#include <QSet>
#include <QPainterPath>
#include "pixel.h"
#include "featurevector.h"
#include "image.forward.h"

class QGraphicsItem;

class Segment {

public:
   Segment();
   Segment(Color const & color, QList<Pixel *> pixels);
   ~Segment();

   Segment & translate(Position vec);
   Segment & rotate(double alpha);

   int area() const;
   double angle() const;
   Position const & position() const;
   Color const & color() const;
   FeatureVector & features();
   FeatureVector const & features() const;
   QSet<Segment *> const & neighbours() const;

   void setPosition(Position const & position);
   void setAngle(double angle);
   void addPixel(Pixel * pixel);
   void addNeighbour(Segment * neighbour);
   void removeNeighbour(Segment * neighbour);
   void merge(Segment * other);

   void calculateColor();
   void relativizePosition();
   void calculateSpatialFeatures();
   void calculateColorFeatures();
   void calculateContour();
   void resetAngle();
   void copyToImage(Image<Color> & image, Position const & offset, bool averageColor = false) const;

   bool collides(Segment const * const other, Position const & offset = Position()) const;
   QGraphicsItem * toQGraphicsItem() const;

private:
   double _angle;
   double _originalAngle;
   double _scale;
   Position _pos;
   Position _minPos;
   Position _maxPos;
   Position _principalAxis;
   Color _color;
   QList<Pixel *> _pixels;
   QSet<Segment *> _neighbours;
   FeatureVector _features;
   QPainterPath contour;

   double calculatePrincipalAxisAngle();
   QPixmap toQPixmap() const;
};

#endif // SEGMENT_H
