// Point.h

#ifndef POINT_H

#define POINT_H

#include <QDebug>
#include "Dim.h"
#include <QPointF>

struct Point {
  Dim x;
  Dim y;
public:
  explicit Point(Dim x=Dim(), Dim y=Dim()): x(x), y(y) { }
  QString toString() const { return x.toString() + " " + y.toString(); }
  static Point fromString(QString s, bool *ok=0);
  QPointF toMils() const { return QPointF(x.toMils(), y.toMils()); }
  static Point fromMils(QPointF const &p) {
    return Point(Dim::fromMils(p.x()), Dim::fromMils(p.y()));
  }
  Point roundedTo(Dim o) const { return Point(x.roundedTo(o),
					      y.roundedTo(o)); }
  Dim distance(Point const &o) const { return Dim::quadrature(o.x-x, o.y-y); }
  Point operator+(Point const &o) const { return Point(x+o.x, y+o.y); }
  Point operator-(Point const &o) const { return Point(x-o.x, y-o.y); }
  Point &operator+=(Point const &o) { x+=o.x; y+=o.y; return *this; }
  Point &operator-=(Point const &o) { x-=o.x; y-=o.y; return *this; }
  Point &operator-() { x=-x; y=-y; return *this; }
  bool operator==(Point const &o) const { return x==o.x && y==o.y; }
  bool operator!=(Point const &o) const { return x!=o.x || y!=o.y; }
  Point flippedLeftRight(Dim x=Dim()) const;
  Point flippedUpDown(Dim y=Dim()) const;
  Point rotatedCW(Point const &p=Point()) const;
  Point rotatedCCW(Point const &p=Point()) const;
  Point &flipLeftRight(Dim x=Dim());
  Point &flipUpDown(Dim y=Dim());
  Point &rotateCW(Point const &p=Point());
  Point &rotateCCW(Point const &p=Point());
  inline static Dim distance(Point const &a, Point const &b) {
    return a.distance(b); }
};


QDebug operator<<(QDebug, Point const &);
inline uint qHash(Point const &p) {
  return qHash(QPair<double,double>(p.x.toMils(),p.y.toMils()));
}


#endif
