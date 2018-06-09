// Point.cpp

#include "Point.h"
#include <QStringList>

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

