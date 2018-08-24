// Polyline.cpp

#include "Polyline.h"
#include "Segment.h"
#include <QPolygonF>

Polyline Polyline::fromString(QString s, bool *ok) {
  Polyline r;
  if (ok)
    *ok = true;
  for (QString bit: s.split(";")) {
    r << Point::fromString(bit, ok);
    if (ok && !*ok)
      return Polyline();
  }
  return r;
}

QString Polyline::toString() const {
  QStringList bits;
  for (Point const &p: *this)
    bits << p.toString();
  return bits.join(";");
}

QPolygonF Polyline::toMils() const {
  QPolygonF pp;
  for (Point const &p: *this)
    pp << p.toMils();
  return pp;
}

bool Polyline::contains(Point p, Dim mrg) const {
  int K = size();
  if (K==0)
    return false;

  // First, see if point is on or near edges
  for (int k=0; k<K-1; k++)
    if (Segment(operator[](k), operator[](k+1)).onSegment(p, mrg))
      return true;
  if (Segment(operator[](0), operator[](K-1)).onSegment(p, mrg))
    return true;

  Segment s0(p, Point(Dim::infinity(), p.y));
  int n = 0;
  bool ok;
  for (int k=0; k<K-1; k++) {
    s0.intersectionWith(Segment(operator[](k), operator[](k+1)), &ok);
    if (ok)
      n++;
  }
  s0.intersectionWith(Segment(operator[](0), operator[](K-1)), &ok);
  if (ok)
    n++;
  
  return (n&1) ? true : false;
}

