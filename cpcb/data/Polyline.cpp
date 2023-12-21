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
  //  bool ok;
  for (int k=0; k<K-1; k++)
    if (s0.intersects(Segment(operator[](k), operator[](k+1))))
      n++;
  if (s0.intersects(Segment(operator[](0), operator[](K-1))))
    n++;
  
  return (n&1) ? true : false;
}

void Polyline::translate(Point const &p1) {
  for (Point &p: *this)
    p += p1;
}

Point Polyline::vertex(int n) const {
  int N = size();
  if (N==0)
    return Point();
  n = n % N;
  if (n<0)
    n += N;
  return operator[](n);
}

void Polyline::setVertex(int n, Point p) {
  int N = size();
  if (N==0)
    return;
  n = n % N;
  if (n<0)
    n += N;
  operator[](n) = p;
}

Segment Polyline::edge(int idx) const {
  return Segment(vertex(idx), vertex(idx+1));
}

bool Polyline::acceptableMove(int idx, Point p) const {
  int N = size();
  if (N<2)
    return false;
  idx = idx % N;
  if (idx<0)
    idx += N;
  Segment a(p, vertex(idx+1));
  Segment b(vertex(idx-1), p);
  for (int tst=idx+2; tst<idx-1+N; tst++)
    if (a.intersects(edge(tst)))
      return false;
  if (edge(idx+1).intersects(b))
    return false;
  if (edge(idx+1).betweenEndpoints(p))
    return false;
  constexpr double LIM = 3.14 * .92; // rather arbitrary...
  if (fabs(a.angle(edge(idx+1))) > LIM)
    return false;
  if (fabs(b.angle(a)) > LIM)
    return false;
  if (fabs(edge(idx-2).angle(b)) > LIM)
    return false;
  return true;
}
