// Geometry.h

#ifndef GEOMETRY_H

#define GEOMETRY_H

#include <QPoint>
#include <QPolygon>
#include <QString>

class Geometry {
public:
  Geometry(class Circuit const &, class PartLibrary const *);
  ~Geometry();
  Geometry(Geometry const &) = delete;
  Geometry operator=(Geometry const &) = delete;
  QPoint pinPosition(int elt, QString pin) const;
  QPolygon connectionPath(int con) const;
private:
  class GeometryData *d;
};

#endif
