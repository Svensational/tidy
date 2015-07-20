#include "segment.h"
#include <QGraphicsPixmapItem>
#include <QPainter>
#include <QPixmap>
#include "image.h"
#include "pixel.h"

Segment::Segment() :
   _angle(0.0), _originalAngle(0.0), _scale(0.75)
{
}

Segment::Segment(Color const & color, QList<Pixel *> pixels) :
   _angle(0.0), _originalAngle(0.0), _scale(0.75), _color(color), _pixels(pixels)
{
}

Segment::~Segment() {
   foreach (Pixel * pixel, _pixels) {
      delete pixel;
   }
   _pixels.clear();
}

void Segment::addNeighbour(Segment * neighbour) {
   if (neighbour && neighbour != this) {
      _neighbours.insert(neighbour);
   }
}

void Segment::addPixel(Pixel * pixel) {
   _pixels << pixel;
}

double Segment::angle() const {
   return _angle + _originalAngle;
}

int Segment::area() const {
   return _pixels.size();
}

void Segment::calculateColor() {
   _color = Color();
   if (!_pixels.isEmpty()) {
      foreach (Pixel const * pixel, _pixels) {
         _color += pixel->col;
      }
      _color /= double(_pixels.size());
   }
}

void Segment::calculateColorFeatures() {
   // get color chanels
   _features[LIGHTNESS] = _color.l99;
   _features[GREENRED] = _color.a99;
   _features[YELLOWBLUE] = _color.b99;
   _features[CROMA] = _color.c();
   _features[HUE] = _color.h();

   // calculate color standard deviation
   _features[COLORSD] = 0.0;
   foreach (Pixel const * const pixel, _pixels) {
      _features[COLORSD] += (pixel->col-_color).magnitudeSquared();
   }
   _features[COLORSD] = sqrt(_features[COLORSD] / double(_pixels.size()));
}

void Segment::calculateContour() {
   // Paint the segment into a QPixmap
   QPixmap pixmap = toQPixmap();

   // create a QGraphicsPixmapItem
   QGraphicsPixmapItem * pixmapItem = new QGraphicsPixmapItem(pixmap);
   pixmapItem->setOffset(_minPos.x, _minPos.y);
   contour = pixmapItem->shape();
}

double Segment::calculatePrincipalAxisAngle() {
   // calculate covariance matrix
   double covar[4]{0.0, 0.0, 0.0, 0.0};
   foreach (Pixel const * const pixel, _pixels) {
      covar[0] += pixel->pos.x * pixel->pos.x;
      covar[1] += pixel->pos.x * pixel->pos.y;
      covar[3] += pixel->pos.y * pixel->pos.y;
   }
   covar[2] = covar[1];
   for (int i=0; i<4; ++i) {
      covar[i] /= _pixels.size();
   }

   // calculate biggest eigenvalue
   double const p_2 = (-covar[0]-covar[3]) / 2.0;
   double const q = covar[0]*covar[3] - covar[1]*covar[2];
   double const lambda = -p_2 + sqrt(p_2*p_2 - q);

   // calculate principal axis
   _principalAxis = Position(-covar[1]/(covar[0]-lambda), 1.0).normalized();

   // calculate principal axis angle
   _originalAngle = acos(_principalAxis.x);
   //_angle = -_originalAngle;
   return acos(_principalAxis.x);
}

void Segment::calculateSpatialFeatures() {
   // get size
   _features[SIZE] = log(_pixels.size());

   // calculate spatial standard deviation
   _features[SPATIALSD] = 0.0;
   foreach (Pixel const * const pixel, _pixels) {
      _features[SPATIALSD] += pixel->pos.magnitudeSquared();
   }
   _features[SPATIALSD] = sqrt(_features[SPATIALSD]/double(_pixels.size()));

   // calculate compactness
   _features[COMPACTNESS] = sqrt(0.14848806049599*_pixels.size()) / _features[SPATIALSD];

   _features[ANGLE] = calculatePrincipalAxisAngle();
}

bool Segment::collides(Segment const * const other, Position const & offset) const {
   QTransform transA;
   transA.translate(_pos.x, _pos.y);
   transA.translate(offset.x, offset.y);
   transA.rotate(_angle * 57.295779513);
   transA.scale(_scale, _scale);

   QTransform transB;
   transB.translate(other->_pos.x, other->_pos.y);
   transB.rotate(other->_angle * 57.295779513);
   transB.scale(other->_scale, other->_scale);

   //QPainterPath pathA = transA.map(a->shape());
   QPainterPath pathA = transA.map(contour);
   QPainterPath pathB = transB.map(other->contour);

   return pathA.intersects(pathB);
}

Color const & Segment::color() const {
   return _color;
}

void Segment::copyToImage(Image<Color> & image, Position const & offset, bool averageColor) const {
   foreach (Pixel const * pixel, _pixels) {
      image.at(qRound(pixel->pos.x + _pos.x + offset.x),
               qRound(pixel->pos.y + _pos.y + offset.y))
            = averageColor?_color:pixel->col;
   }
}

FeatureVector & Segment::features() {
   return _features;
}

FeatureVector const & Segment::features() const {
   return _features;
}

void Segment::merge(Segment * other) {
   // merge the other segment into this
   _color = (_color*area() + other->_color*other->area())/double(area()+other->area());
   _pixels.append(other->_pixels);  // position is not taken into account!
   _neighbours.unite(other->_neighbours);
   _neighbours.remove(this);
   _neighbours.remove(other);

   // update neighbours
   foreach (Segment * neighbour, other->neighbours()) {
      neighbour->removeNeighbour(other);
      neighbour->addNeighbour(this);
   }

   // clean the other segment
   other->_pixels.clear();
   other->_neighbours.clear();
}

const QSet<Segment *> & Segment::neighbours() const {
   return _neighbours;
}

Position const & Segment::position() const {
   return _pos;
}

void Segment::relativizePosition() {
   // calculate center
   _pos = Position();
   foreach (Pixel const * const pixel, _pixels) {
      _pos += pixel->pos;
   }
   _pos /= _pixels.size();

   // convert pixel positions from absolute to relative
   foreach (Pixel * const pixel, _pixels) {
      pixel->pos -= _pos;
   }

   // determin minimal and maximal positions (relative)
   _minPos = Position();
   _maxPos = Position();
   foreach (Pixel const * const pixel, _pixels) {
      _minPos.x = std::min(_minPos.x, pixel->pos.x);
      _minPos.y = std::min(_minPos.y, pixel->pos.y);
      _maxPos.x = std::max(_maxPos.x, pixel->pos.x);
      _maxPos.y = std::max(_maxPos.y, pixel->pos.y);
   }
}

void Segment::removeNeighbour(Segment * neighbour) {
   _neighbours.remove(neighbour);
}

void Segment::resetAngle() {
   _angle = 0.0;
}

Segment & Segment::rotate(double alpha) {
   _angle += alpha;
   return *this;
}

void Segment::setAngle(double angle) {
   _angle = angle - _originalAngle;
}

void Segment::setPosition(Position const & position) {
   _pos = position;
}

QGraphicsItem * Segment::toQGraphicsItem() const {
   // Paint the segment into a QPixmap
   QPixmap pixmap = toQPixmap();

   // create a QGraphicsPixmapItem
   QGraphicsPixmapItem * pixmapItem = new QGraphicsPixmapItem(pixmap);
   pixmapItem->setTransformationMode(Qt::SmoothTransformation);
   pixmapItem->setOffset(_minPos.x, _minPos.y);
   pixmapItem->setPos(_pos.x, _pos.y);
   pixmapItem->setRotation(_angle*57.295779513);
   pixmapItem->setScale(_scale);

   return pixmapItem;
}

QPixmap Segment::toQPixmap() const {
   QPixmap pixmap(_maxPos.x-_minPos.x+1, _maxPos.y-_minPos.y+1);
   pixmap.fill(QColor(0, 0, 0, 0));
   QPainter painter(&pixmap);
   foreach (Pixel const * pixel, _pixels) {
      painter.setPen(QColor(pixel->col.toQRgb()));
      painter.drawPoint(qRound(pixel->pos.x - _minPos.x),
                        qRound(pixel->pos.y - _minPos.y));
   }
   return pixmap;
}

Segment & Segment::translate(Position vec) {
   _pos += vec;
   return *this;
}
