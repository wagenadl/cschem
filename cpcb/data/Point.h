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
  bool isNull() const;
  Point operator+(Point const &o) const { return Point(x+o.x, y+o.y); }
  Point operator-(Point const &o) const { return Point(x-o.x, y-o.y); }
  Point &operator+=(Point const &o) { x+=o.x; y+=o.y; return *this; }
  Point &operator-=(Point const &o) { x-=o.x; y-=o.y; return *this; }
  Point operator-() const { Point q(-x, -y); return q; }
  bool operator==(Point const &o) const { return x==o.x && y==o.y; }
  bool operator!=(Point const &o) const { return x!=o.x || y!=o.y; }
  Point operator*(qint64 a) const { return Point(x*a, y*a); }
  Point operator/(qint64 a) const { return Point(x/a, y/a); }
  Point flippedLeftRight(Dim x=Dim()) const;
  Point flippedUpDown(Dim y=Dim()) const;
  Point rotatedCW(Point const &p=Point()) const;
  Point rotatedCCW(Point const &p=Point()) const;
  Point &flipLeftRight(Dim x=Dim());
  Point &flipUpDown(Dim y=Dim());
  Point &rotateCW(Point const &p=Point());
  Point &rotateCCW(Point const &p=Point());
  Point &freeRotate(int degreesCW, Point const &p=Point());
  Point rotatedFreely(int degreesCW, Point const &p=Point()) const;
  inline static Dim distance(Point const &a, Point const &b) {
    return a.distance(b); }
  bool operator<(Point const &o) const { return y==o.y ? x<o.x : y<o.y; }
  qint64 innerProduct(Point const &b) const;
};


QDebug operator<<(QDebug, Point const &);
inline uint qHash(Point const &p) {
  return qHash(QPair<double,double>(p.x.toMils(),p.y.toMils()));
}


#endif
