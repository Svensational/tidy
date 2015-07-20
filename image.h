#ifndef IMAGE_H
#define IMAGE_H

#include <QImage>
#include "image.forward.h"

template <typename C>
class Image {

public:
   Image() :
      _width(0), _height(0), _data(nullptr)
   {
   }

   Image(int width, int height) :
      _width(width), _height(height), _data(new C[width*height])
   {
      memset(_data, 0x00, width*height*sizeof(C));
   }

   Image(Image<C> && other) :
      _width(other._width), _height(other._height), _data(other._data)
   {
      other._data = nullptr;
   }

   explicit Image(QString const & filename) :
      _width(0), _height(0), _data(nullptr)
   {
      QImage image(filename);
      if (!image.isNull()) {
         if (image.format() != QImage::Format_RGB32) {
            image = image.convertToFormat(QImage::Format_RGB32);
         }
         _width = image.width();
         _height = image.height();
         _data = new C[_width*_height];
         QRgb const * pixel = reinterpret_cast<QRgb const *>(image.constBits());
         for (int i=0; i<area(); ++i) {
            _data[i] = C(*pixel++);
         }
      }
   }

   ~Image() {
      delete[] _data;
   }

   Image & operator=(Image && other) {
      _width = other._width;
      _height = other._height;
      std::swap(_data, other._data);
      return *this;
   }


   bool isNull() const {
      return (_width == 0 || _height == 0 || !_data);
   }

   int width() const {
      return _width;
   }

   int height() const {
      return _height;
   }

   int area() const {
      return _width * _height;
   }

   int maxWH() const {
      return std::max(_width, _height);
   }


   C & at(int x, int y) {
      return _data[y*_width + x];
   }

   C const & at(int x, int y) const {
      return _data[y*_width + x];
   }

   C & at(int i) {
      return _data[i];
   }

   C const & at(int i) const {
      return _data[i];
   }


   void fill(C color) {
      for (int i=0; i<area(); ++i) {
         _data[i] = color;
      }
   }


   QImage toQImage() const {
      QImage image(_width, _height, QImage::Format_RGB32);
      QRgb * pixel = reinterpret_cast<QRgb *>(image.bits());
      for (int i=0; i<_width*_height; ++i) {
         pixel[i] = _data[i].toQRgb();
      }
      return image;
   }

   void save(QString const & filename) const {
      toQImage().save(filename);
   }

   inline bool areNeighbours(int indexA, int indexB) const {
      return (indexA >= 0 && indexA < _width*_height &&
              indexB >= 0 && indexB < _width*_height &&
              std::abs(indexA % _width - indexB % _width) <= 1 &&
              std::abs(indexA / _width - indexB / _width) <= 1);
   }

private:
   int _width;
   int _height;
   C * _data;
};

#endif // IMAGE_H
