// Connection.h

#ifndef CONNECTION_H

#define CONNECTION_H

#include <QSharedData>
#include <QXmlStreamReader>
#include "PinID.h"
#include <QPolygon>

class Connection {
public:
  Connection();
  Connection(Connection const &);
  Connection(QXmlStreamReader &src);
  Connection &operator=(Connection const &);
  ~Connection();
  QString report() const;
public:
  int id() const;
  int fromId() const; // zero if dangling
  int toId() const; // zero if dangling
  QString fromPin() const;
  QString toPin() const;
  QPolygon const &via() const;
  PinID from() const;
  PinID to() const;
  bool isEquivalentTo(Connection const &) const;
  bool isNull() const;
  bool isDangling() const;
  bool isCircular() const;
  Connection reversed() const; // does *not* assign a new ID
  Connection translated(QPoint delta) const;
public:
  void setFrom(PinID);
  void setTo(PinID);
  void setId(int);
  void setFromId(int);
  void setToId(int);
  void setFromPin(QString);
  void setToPin(QString);
  void setFrom(int id, QString pin);
  void setTo(int id, QString pin);
  QPolygon &via();
  void setVia(QVector<QPoint> const &);
  void translate(QPoint delta);
  void reverse();
private:
  QSharedDataPointer<class ConnectionData> d;
};

QXmlStreamWriter &operator<<(QXmlStreamWriter &, Connection const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Connection &);


#endif
