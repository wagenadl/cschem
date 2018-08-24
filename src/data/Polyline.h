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
};

#endif
