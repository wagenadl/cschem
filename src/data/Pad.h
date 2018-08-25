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
  bool topfpcon;
  bool bottomfpcon;
public:
  Pad();
  bool isValid() const { return layer!=Layer::Invalid; }
  Rect boundingRect() const;
  void rotate();
  Point intersectionWith(class Trace const &t, bool *ok=0) const;
  /* Returns our center if t touches us, otherwise null. */
};

QDebug operator<<(QDebug, Pad const &);
QXmlStreamWriter &operator<<(QXmlStreamWriter &, Pad const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Pad &);

#endif
