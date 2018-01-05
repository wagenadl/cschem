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
  QPoint pinPosition(class PinID const &) const;
  QPoint pinPosition(int elt, QString pin) const;
  QPoint pinPosition(class Element const &elt, QString pin) const;
  QPoint centerOfPinMass(int elt) const;
  QPoint centerOfPinMass(class Element const &elt) const;
  QPoint centerOfPinMass() const; // for whole circuit
  QPolygon connectionPath(int con) const;
  QPolygon connectionPath(class Connection const &con) const;
  bool isZeroLength(int con) const;
  bool isZeroLength(class Connection const &con) const;
  /* Invalid connections are zero length by definition. */
  struct Intersection {
    Intersection(int n=-1, QPoint d=QPoint()):
      pointnumber(n), delta(d) {
    }
    int pointnumber;
    QPoint delta;
  };
  static Intersection intersection(QPoint p, QPolygon poly);
  /* Returns information about the point on the polygon that is closest to
     the target point. The result is a structure that encodes the point
     number (index into the polygon) as well as a vector away from that point.
     The point Q := poly[pointnumber] + delta will actually lie on 
     the polygon if all segments in poly are horizontal or vertical. If there
     are diagonal segments, Q will be as near as possible.
     The distance between P and Q can be used to check if P is
     actually on or near the polygon. The polygon is considered an
     open path unless its first and last point are equal.
     If POLY is empty, the result will have pointnumber=-1.
  */
public:
  static QPolygon simplifiedPath(QPolygon path);
  static QPolygon viaFromPath(class Connection const &con, QPolygon path);
  QPolygon viaFromPath(int con, QPolygon path) const;
private:
  class GeometryData *d;
};

#endif
