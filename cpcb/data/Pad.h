// Pad.h

#ifndef PAD_H

#define PAD_H

#include "Rect.h"
#include "FreeRotation.h"
#include <QXmlStreamReader>
#include <QDebug>
#include "Layer.h"

class Pad {
public:
  Point p; // center
  Dim width;
  Dim height;
  FreeRotation rota;
  Layer layer;
  bool elliptic;
  QString ref;
  bool fpcon;
  bool noclear;
public:
  Pad();
  bool isValid() const { return layer!=Layer::Invalid; }
  Rect boundingRect() const;
  void rotateCW(); // around our center
  bool touches(class Trace const &t) const;
  bool touches(class FilledPlane const &fp) const;
  void rotateCW(Point const &p0);
  void flipLeftRight(Dim const &x0);
  void flipUpDown(Dim const &y0);
};

QDebug operator<<(QDebug, Pad const &);
QXmlStreamWriter &operator<<(QXmlStreamWriter &, Pad const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Pad &);

#endif
