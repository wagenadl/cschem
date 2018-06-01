// Point.h

#ifndef POINT_H

#define POINT_H

#include <QDebug>
#include "Dim.h"

struct Point {
  Dim x;
  Dim y;
public:
  explicit Point(Dim x=Dim(), Dim y=Dim()): x(x), y(y) { }
  QString toString() const { return x.toString() + " " + y.toString(); }
  static Point fromString(QString s, bool *ok=0);
};

QDebug operator<<(QDebug, Point const &);


#endif
