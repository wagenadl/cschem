// Polyline.cpp

#include "Polyline.h"

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
