// Hole.h

#ifndef HOLE_H

#define HOLE_H

#include "Rect.h"
#include "Layer.h"
#include "FreeRotation.h"
#include <QXmlStreamReader>
#include <QDebug>
#include "Segment.h"
#include <QPainterPath>

class Hole {
public:
  Point p;
  Dim id;
  Dim od;
  Dim slotlength;
  FreeRotation rota;
  bool square;
  bool via;
  QString ref;
  Layer fpcon;
  bool noclear;
public:
  Hole();
  bool isValid() const { return !od.isNull(); }
  Rect boundingRect() const;
  /* The "touches" functions do not yet take slotting properly into account.
     Or filled plane connections.
   */
  QPainterPath outlinePath(Layer l=Layer::Invalid) const;
  bool touches(class Trace const &t) const;
  bool touches(class FilledPlane const &fp) const;
  bool touches(class Pad const &p) const;
  bool touches(class Hole const &p) const;
  void rotateCW(Point const &p0);
  void freeRotate(FreeRotation const &degcw, Point const &p0);
  void flipLeftRight(Dim const &x0);
  void flipUpDown(Dim const &y0);
  bool isSlot() const;
  Segment slotEnds() const;
};

QDebug operator<<(QDebug, Hole const &);
QXmlStreamWriter &operator<<(QXmlStreamWriter &, Hole const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Hole &);

#endif
