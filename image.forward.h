#ifndef IMAGE_FORWARD_H
#define IMAGE_FORWARD_H

#include "color.h"
#include "gray.h"

template <typename C> class Image;

using ImageColor = Image<Color>;
using ImageGray = Image<Gray>;

#endif // IMAGE_FORWARD_H
