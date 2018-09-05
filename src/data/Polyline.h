// Polyline.h

#ifndef POLYLINE_H

#define POLYLINE_H

#include "Point.h"
#include <QVector>

class Polyline: public QVector<Point> {
public:
  static Polyline fromString(QString, bool *ok=0);
  QString toString() const;
  QPolygonF toMils() const;
  bool contains(Point p, Dim mrg=Dim()) const;
  void translate(Point const &);
  bool selfIntersects(int idx) const; // check given vertex and its edges
};

#endif
