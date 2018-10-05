// Nphole.h

#ifndef NPHOLE_H

#define NPHOLE_H

#include "Rect.h"
#include <QXmlStreamReader>
#include <QDebug>
#include "FreeRotation.h"

class NPHole {
public:
  Point p;
  Dim d;
  Dim slotlength;
  FreeRotation rota;
public:
  NPHole();
  bool isValid() const { return !d.isNull(); }
  Rect boundingRect() const;
};

QDebug operator<<(QDebug, NPHole const &);
QXmlStreamWriter &operator<<(QXmlStreamWriter &, NPHole const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, NPHole &);

#endif
