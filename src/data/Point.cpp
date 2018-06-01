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

QDebug operator<<(QDebug dbg, Point const &p) {
  dbg << "(" << p.x << "," << p.y << ")";
  return dbg;
}

