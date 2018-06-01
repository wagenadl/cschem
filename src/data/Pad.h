// Pad.h

#ifndef PAD_H

#define PAD_H

#include "Point.h"
#include <QXmlStreamReader>
#include <QDebug>
#include "Layer.h"

class Pad {
public:
  Point p;
  Dim width;
  Dim height;
  Layer layer;
public:
  Pad();
  bool isValid() const { return layer!=Layer::Invalid; }
};

QDebug operator<<(QDebug, Pad const &);
QXmlStreamWriter &operator<<(QXmlStreamWriter &, Pad const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Pad &);

#endif
