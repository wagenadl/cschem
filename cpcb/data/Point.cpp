// Point.cpp

#include "Point.h"
#include <QStringList>
#include "pi.h"

Point Point::fromString(QString s, bool *ok) {
  Point p;
  bool ok1 = false;
  QStringList l = s.split(" ");
  if (l.size()==2) {
    p.x = Dim::fromString(l[0], &ok1);
    if (ok1)
      p.y = Dim::fromString(l[1], &ok1);
  }
  if (ok)
    *ok = ok1;
  return p;
}

Point Point::flippedLeftRight(Dim x0) const {
  return Point(2*x0 - x, y);
}
  
Point Point::flippedUpDown(Dim y0) const {
  return Point(x, 2*y0 - y);
}

Point Point::rotatedFreely(int deg, Point const &p0) const {
  Point p = *this;
  p.freeRotate(deg, p0);
  return p;
}

Point &Point::freeRotate(int deg, Point const &p0) {
  double phi = deg*PI/180;
  double cs = cos(phi);
  double sn = sin(phi);
  x -= p0.x;
  y -= p0.y;
  Dim x1 = x*cs - y*sn;
  Dim y1 = x*sn + y*cs;
  x = x1 + p0.x;
  y = y1 + p0.y;
  return *this;
}

Point Point::rotatedCW(Point const &p0) const {
  Point p = *this - p0;
  Point q(-p.y, p.x);
  return q + p0;
}

Point Point::rotatedCCW(Point const &p0) const {
  Point p = *this - p0;
  Point q(p.y, -p.x);
  return q + p0;
}

Point &Point::flipLeftRight(Dim x0) {
  x = 2*x0 - x;
  return *this;
}
  
Point &Point::flipUpDown(Dim y0) {
  y = 2*y0 - y;
  return *this;
}

Point &Point::rotateCW(Point const &p0) {
  Point p = *this - p0;
  Point q(-p.y, p.x);
  *this = q + p0;
  return *this;
}

Point &Point::rotateCCW(Point const &p0) {
  Point p = *this - p0;
  Point q(p.y, -p.x);
  *this = q + p0;
  return *this;
}


QDebug operator<<(QDebug dbg, Point const &p) {
  dbg << "(" << p.x << "," << p.y << ")";
  return dbg;
}

bool Point::isNull() const {
  return x.isNull() && y.isNull();
}

qint64 Point::innerProduct(Point const &b) const {
  return x.raw() * b.x.raw()  + y.raw() * b.y.raw();
}

Point Point::average(QSet<Point> const &pp) {
  Point p;
  for (Point const &p1: pp)
    p += p1;
  return p / pp.size();
}

