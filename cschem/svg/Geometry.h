// Geometry.h

#ifndef GEOMETRY_H

#define GEOMETRY_H

#include <QPoint>
#include <QPolygon>
#include <QString>
#include "circuit/PinID.h"

class Geometry {
public:
  Geometry(class Circuit const &, class SymbolLibrary const &);
  ~Geometry();
  Geometry();
  Geometry(Geometry const &) = delete;
  Geometry &operator=(Geometry const &) = delete;
  Geometry(Geometry &&);
  Geometry &operator=(Geometry &&);
  QPoint pinPosition(class PinID const &) const;
  QPoint pinPosition(int elt, QString pin) const;
  QPoint pinPosition(class Element const &elt, QString pin) const;
  QMap<QString, QPoint> pinPositions(int elt) const;
  QMap<QString, QPoint> pinPositions(class Element const &elt) const;
  QPoint preferredRoutingDirection(class PinID const &) const;
  QPoint preferredRoutingDirection(int elt, QString pin) const;
  QPoint preferredRoutingDirection(Element const &elt, QString pin) const;
  /* PREFERREDROUTINGDIRECTION - Which we to route from pin given choice
     (1,0) means to the right
     (-1,0) means to the left
     (0,1) means down
     (0,-1) means up
  */
  QPoint centerOfPinMass(int elt) const;
  QPoint centerOfPinMass(class Element const &elt) const;
  QPoint centerOfPinMass() const; // for whole circuit
  QPolygon connectionPath(int con) const;
  QPolygon connectionPath(class Connection const &con) const;
  bool isZeroLength(int con) const;
  bool isZeroLength(class Connection const &con) const;
  /* Invalid connections are zero length by definition. */
  QRectF visualBoundingRect(class Element const &elt) const;
  QRectF visualBoundingRect() const; // for whole circuit
  QRectF svgBoundingRect(class Element const &elt) const;
  /* Bounding box of (rotated and flipped) symbol relative to its first pin. */
  QRectF defaultAnnotationSvgBoundingRect(class Element const &elt,
					  QString annotation) const;
  /* This is the bounding rectangle, in svg coordinates rather than circuit
     coordinates, relative to the origin (first pin) of the element, of the
     named annotation ("name" or "value" in the present implementation) as
     defined by the svg. When the svg doesn't define a bbox, a reasonable
     suggestion is returned. */
  struct Intersection {
    Intersection(int n=-1, QPoint q=QPoint()):
      index(n), q(q) {
    }
    int index;
    QPoint q;
  };
  Intersection intersection(QPoint p, int con, bool nodiag=false);
  Intersection intersection(QPoint p, Connection const &con, bool nodiag=false);
  static Intersection intersection(QPoint p, QPolygon poly, bool nodiag=false);
  /* Returns information about the point on the polygon that is closest to
     the target point. The result is a structure that encodes the point
     number (index into the polygon) as well as a vector away from that point.
     The point Q will actually lie on 
     the polygon if all segments in poly are horizontal or vertical. If there
     are diagonal segments, Q will be as near as possible.
     The distance between P and Q can be used to check if P is
     actually on or near the polygon. The polygon is considered an
     open path unless its first and last point are equal.
     If POLY is empty, the result will have INDEX=-1, otherwise, INDEX is
     an index into poly that says to which segment Q belongs.
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
