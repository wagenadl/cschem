// Polyline.h

#ifndef POLYLINE_H

#define POLYLINE_H

#include "Point.h"
#include "Segment.h"
#include <QVector>

class Polyline: public QVector<Point> {
public:
  static Polyline fromString(QString, bool *ok=0);
  QString toString() const;
  QPolygonF toMils() const;
  bool contains(Point p, Dim mrg=Dim()) const;
  void translate(Point const &);
  bool acceptableMove(int idx, Point p) const;
  Point vertex(int idx) const; // idx is taken mod-N.
  Segment edge(int idx) const; // segment following given edge.
  void setVertex(int idx, Point); // idx is taken mod-N.
};

#endif
