// Hole.h

#ifndef HOLE_H

#define HOLE_H

#include "Rect.h"
#include "Layer.h"
#include <QXmlStreamReader>
#include <QDebug>

class Hole {
public:
  Point p;
  Dim id;
  Dim od;
  bool square;
  QString ref;
  Layer fpcon;
  bool noclear;
public:
  Hole();
  bool isValid() const { return !od.isNull(); }
  Rect boundingRect() const;
  bool touches(class Trace const &t) const;
  bool touches(class FilledPlane const &fp) const;
};

QDebug operator<<(QDebug, Hole const &);
QXmlStreamWriter &operator<<(QXmlStreamWriter &, Hole const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Hole &);

#endif
