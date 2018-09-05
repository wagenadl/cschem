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

void Polyline::translate(Point const &p1) {
  for (Point &p: *this)
    p += p1;
}

bool Polyline::selfIntersects(int idx) const {
  int N = size();
  if (idx<0 || idx>=N || N<2)
    return false; // hmmm.
  QVector<Point> const &pp(*this);
  Segment a(pp[idx], pp[(idx+1)%N]);
  for (int tst=idx+2; tst<idx-1+N; tst++) {
    Segment s(pp[tst%N], pp[(tst+1)%N]);
    if (a.touches(s)) 
      return true;
  }
  if (Segment(pp[(idx+1)%N], pp[(idx+2)%N])
      .touches(Segment(pp[(idx+N-1)%N], pp[idx])))
    return true;
  if (Segment(pp[(idx+1)%N], pp[(idx+2)%N]).betweenEndpoints(pp[idx]))
    return true;
  if (Segment(pp[(idx+N-1)%N], pp[idx%N]).betweenEndpoints(pp[idx]))
    return true;
  return false;
}
