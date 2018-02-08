// Connection.h

#ifndef CONNECTION_H

#define CONNECTION_H

#include <QXmlStreamReader>
#include "PinID.h"
#include <QPolygon>
#include <QDebug>

class Connection {
public:
  Connection();
  QString report() const;
  Connection(PinID, PinID);
public:
  PinID from() const;
  PinID to() const;
  bool isEquivalentTo(Connection const &) const;
  bool isValid() const;
  /* Valid means either:
     (1) Two endpoints on pins that are not the same
     (2) One endpoint on a pin and at least one point in via
     (3) No endpoints on a pin but at least two points in via, _and_ via
         not starting and ending at same spot.
     Valid cannot test for zero-length in case (2). For that, use
     Geometry::isZeroLength(). */
  bool isDangling() const; // either start or end is dangling
  bool danglingStart() const; // start not connected to pin
  bool danglingEnd() const; // end not connected to pin
  Connection reversed() const; // does *not* assign a new ID
  Connection translated(QPoint delta) const;
public:
  void setFrom(PinID);
  void setTo(PinID);
  void setFrom(int id, QString pin="");
  void setTo(int id, QString pin="");
  void unsetFrom();
  void unsetTo();
  void translate(QPoint delta);
  void reverse();
public:
  int id;
  int fromId;
  int toId;
  QString fromPin;
  QString toPin;
  QPolygon via;
};

QXmlStreamWriter &operator<<(QXmlStreamWriter &, Connection const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Connection &);
QDebug &operator<<(QDebug &, Connection const &);

#endif
