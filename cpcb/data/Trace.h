// Trace.h

#ifndef TRACE_H

#define TRACE_H

#include "Segment.h"
#include "Rect.h"
#include "Layer.h"
#include <QXmlStreamReader>
#include <QDebug>
#include <QPainterPath>

class Trace: public Segment {
public:
  Layer layer;
  Dim width;
  bool noclear;
public:
  Trace();
  bool isValid() const { return layer!=Layer::Invalid; }
  Rect boundingRect() const;
  QPainterPath outlinePath() const;
  bool onP1(Point p, Dim mrg=Dim()) const;
  // ONP1(p, mrg) returns true if P is close enough to P1.
  // Close enough means within a distance .5*WIDTH + MRG.
  bool onP2(Point p, Dim mrg=Dim()) const;
  bool onSegment(Point p, Dim mrg=Dim()) const;
  // ONSEGMENT(p, mrg) returns true if P is close enough to the line segment.
  // Close enough means within a distance .5*WIDTH + MRG.
  // ONSEGMENT returns true even if ONP1 or ONP2 would return true as well.
  bool touches(Trace const &t, Point *intersection=0) const;
  bool touches(class FilledPlane const &fp) const;
  bool touches(Rect r) const;
  bool operator==(Trace const &) const;
};

QDebug operator<<(QDebug, Trace const &);
QXmlStreamWriter &operator<<(QXmlStreamWriter &, Trace const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Trace &);

#endif
