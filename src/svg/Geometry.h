// Geometry.h

#ifndef GEOMETRY_H

#define GEOMETRY_H

#include <QPoint>
#include <QPolygon>
#include <QString>
#include  "file/PinID.h"

class Geometry {
public:
  Geometry(class Circuit const &, class PartLibrary const *);
  ~Geometry();
  Geometry();
  Geometry(Geometry const &) = delete;
  Geometry &operator=(Geometry const &) = delete;
  Geometry(Geometry &&);
  Geometry &operator=(Geometry &&);
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
  QRect boundingRect(int elt) const;
  QRect boundingRect(class Element const &elt) const;
  QRect boundingRect() const; // for whole circuit
  /* boundingRect is naive about annotations */
  struct Intersection {
    Intersection(int n=-1, QPoint d=QPoint()):
      pointnumber(n), delta(d) {
    }
    int pointnumber;
    QPoint delta;
  };
  static Intersection intersection(QPoint p, QPolygon poly, bool nodiag=false);
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
     If NODIAG is true, segments that are not perfectly horizonal or
     vertical are ignored.
  */
  int connectionAt(QPoint p) const;
  /* Finds out whether there is any connection passing through the
     point P and returns its identity. The first call is very slow,
     since a database of all points on connections has to be
     constructed. Subsequent calls are fast. Connection segments that
     are not perfectly horizontal or vertical are ignored.
   */
  PinID pinAt(QPoint p) const;
  /* Finds out whether there is a pin at the point P and returns its
     identity. The first call is slow, because we have to set up a
     database of pin locations, but subsequent calls are fast. */
public:
  static QPolygon simplifiedPath(QPolygon path);
  static QPolygon viaFromPath(class Connection const &con, QPolygon path);
  QPolygon viaFromPath(int con, QPolygon path) const;
private:
  class GeometryData *d;
};

#endif
