// Connection.cpp

#include "Connection.h"
#include "IDFactory.h"
#include <QPoint>
#include <QDebug>

class ConnectionData: public QSharedData {
public:
  ConnectionData(): id(IDFactory::instance().newId()) {
    fromId = toId = 0;
  }
public:
  int id;
  int fromId;
  int toId;
  QString fromPin;
  QString toPin;
  QPolygon via;
};

bool Connection::isNull() const {
  return d->fromId==0 && d->toId==0
    && d->fromPin.isEmpty() && d->toPin.isEmpty()
    && d->via.isEmpty();
}

bool Connection::isDangling() const {
  return d->fromId<=0 || d->toId<=0;
}

bool Connection::isCircular() const {
  return (d->fromId>0 && d->fromId==d->toId)
    || ((d->fromId<=0 || d->toId<=0) && d->via.isEmpty())
    || (d->fromId<=0 && d->toId<=0
        && (d->via.isEmpty() || d->via.first()==d->via.last()));
}

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

QPolygon const &Connection::via() const {
  return d->via;
}

void Connection::setId(int id) {
  d.detach();
  d->id = id;
}

void Connection::setFromId(int id) {
  d.detach();
  d->fromId = id;
}

void Connection::setToId(int id) {
  d.detach();
  d->toId = id;
}

void Connection::setFromPin(QString pin) {
  d.detach();
  d->fromPin = pin;
}

void Connection::setToPin(QString pin) {
  d.detach();
  d->toPin = pin;
}

void Connection::setFrom(int id, QString pin) {
  d.detach();
  d->fromId = id;
  d->fromPin = pin;
}

void Connection::setTo(int id, QString pin) {
  d.detach();
  d->toId = id;
  d->toPin = pin;
}

QPolygon &Connection::via() {
  d.detach();
  return d->via;
}

void Connection::setVia(QVector<QPoint> const &v) {
  d.detach();
  d->via = v;
}

Connection Connection::translated(QPoint delta) const {
  Connection c = *this;
  c.translate(delta);
  return c;
}

void Connection::translate(QPoint delta) {
  d.detach();
  for (QPoint &p: d->via)
    p += delta;
}

Connection Connection::reversed() const {
  Connection con = *this;
  QPolygon v;
  for (auto p: via())
    v.prepend(p);
  con.setFrom(to());
  con.setTo(from());
  con.setVia(v);
  return con;
}

void Connection::reverse() {
  d.detach();
  *this = reversed();
}

PinID Connection::from() const {
  return PinID(d->fromId, d->fromPin);
}

PinID Connection::to() const {
  return PinID(d->toId, d->toPin);
}

void Connection::setFrom(PinID pp) {
  d.detach();
  d->fromId = pp.element();
  d->fromPin = pp.pin();
}

void Connection::setTo(PinID pp) {
  d.detach();
  d->toId = pp.element();
  d->toPin = pp.pin();
}

bool Connection::isEquivalentTo(Connection const &o) const {
  return (from()==o.from() && to()==o.to())
    ||  (from()==o.to() && to()==o.from());
}

QString Connection::report() const {
  QString x = QString("%1 = %2:%3 - ").arg(id()).arg(fromId()).arg(fromPin());
  for (auto v: via())
    x += QString("%1,%2 ").arg(v.x()).arg(v.y());
  x += QString("- %1:%2").arg(toId()).arg(toPin());
  return x;
}

