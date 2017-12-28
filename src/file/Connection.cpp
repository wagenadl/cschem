// Connection.cpp

#include "Connection.h"
#include "IDFactory.h"
#include <QPoint>

class ConnectionData: public QSharedData {
public:
  ConnectionData(): id(IDFactory::instance().newId()) { }
public:
  int id;
  int fromId;
  int toId;
  QString fromPin;
  QString toPin;
  QList<QPoint> via;
};

Connection::Connection() {
  d = new ConnectionData();
}

Connection::Connection(Connection const &o) {
  d = o.d;
}

Connection &Connection::operator=(Connection const &o) {
  d = o.d;
  return *this;
}

Connection::~Connection() {
}


Connection::Connection(QXmlStreamReader &src): Connection() {
  auto a = src.attributes();
  d->id = a.value("id").toInt();
  QStringList from = a.value("from").toString().split(":");
  if (from.size()>=1) 
    d->fromId = from[0].toInt();
  if (from.size()>=2)
    d->fromPin = from[1];
  QStringList to = a.value("to").toString().split(":");
  if (to.size()>=1) 
    d->toId = to[0].toInt();
  if (to.size()>=2)
    d->toPin = to[1];
  QStringList via = a.value("via").toString().split(" ");
  for (QString &v: via) {
    QStringList xy = v.split(",");
    if (xy.size()==2) 
      d->via << QPoint(xy[0].toInt(), xy[1].toInt());
  }
  src.skipCurrentElement();
}

QXmlStreamReader &operator>>(QXmlStreamReader &sr, Connection &c) {
  c = Connection(sr);
  return sr;
}
  
QXmlStreamWriter &operator<<(QXmlStreamWriter &sr, Connection const &c) {
  sr.writeStartElement("connection");
  sr.writeAttribute("id", QString::number(c.id()));
  sr.writeAttribute("from",
                    QString("%1:%2").arg(c.fromId()).arg(c.fromPin()));
  sr.writeAttribute("to",
                    QString("%1:%2").arg(c.toId()).arg(c.toPin()));
  if (!c.via().isEmpty()) {
    QStringList via;
    for (QPoint const &p: c.via())
      via << QString("%1,%2").arg(p.x()).arg(p.y());
    sr.writeAttribute("via", via.join(" "));
  }
  sr.writeEndElement();
  return sr;
};

int Connection::id() const {
  return d->id;
}

int Connection::fromId() const {
  return d->fromId;
}

int Connection::toId() const {
  return d->toId;
}

QString Connection::fromPin() const {
  return d->fromPin;
}

QString Connection::toPin() const {
  return d->toPin;
}

QList<QPoint> const &Connection::via() const {
  return d->via;
}

void Connection::setId(int id) {
  d->id = id;
}

void Connection::setFromId(int id) {
  d->fromId = id;
}

void Connection::setToId(int id) {
  d->toId = id;
}

void Connection::setFromPin(QString pin) {
  d->fromPin = pin;
}

void Connection::setToPin(QString pin) {
  d->toPin = pin;
}

void Connection::setFrom(int id, QString pin) {
  d->fromId = id;
  d->fromPin = pin;
}

void Connection::setTo(int id, QString pin) {
  d->toId = id;
  d->toPin = pin;
}

QList<QPoint> &Connection::via() {
  return d->via;
}

void Connection::setVia(QList<QPoint> const &v) {
  d->via = v;
}

void Connection::translate(QPoint delta) {
  for (QPoint &p: d->via)
    p += delta;
}
      
