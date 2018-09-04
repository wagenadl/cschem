// Pad.h

#ifndef PAD_H

#define PAD_H

#include "Rect.h"
#include <QXmlStreamReader>
#include <QDebug>
#include "Layer.h"

class Pad {
public:
  Point p; // center
  Dim width;
  Dim height;
  Layer layer;
  bool elliptic;
  QString ref;
  bool fpcon;
  bool noclear;
public:
  Pad();
  bool isValid() const { return layer!=Layer::Invalid; }
  Rect boundingRect() const;
  void rotate();
  bool touches(class Trace const &t) const;
  bool touches(class FilledPlane const &fp) const;
};

QDebug operator<<(QDebug, Pad const &);
QXmlStreamWriter &operator<<(QXmlStreamWriter &, Pad const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Pad &);

#endif
