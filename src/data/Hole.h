// Hole.h

#ifndef HOLE_H

#define HOLE_H

#include "Rect.h"
#include <QXmlStreamReader>
#include <QDebug>

class Hole {
public:
  Point p;
  Dim id;
  Dim od;
  bool square;
public:
  Hole();
  bool isValid() const { return !od.isNull(); }
  Rect boundingRect() const;
};

QDebug operator<<(QDebug, Hole const &);
QXmlStreamWriter &operator<<(QXmlStreamWriter &, Hole const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Hole &);

#endif
