// Connection.cpp

#include "Connection.h"
#include "IDFactory.h"
#include <QPoint>
#include <QDebug>

bool Connection::isValid() const {
  if (fromId>0 && toId>0)
    return fromId!=toId || fromPin!=toPin;
  else if (fromId>0 || toId>0)
    return via.size() >= 1;
  else
    return via.size() >= 2 && via.first() != via.last();
}

bool Connection::isDangling() const {
  return danglingStart() || danglingEnd();
}

bool Connection::danglingStart() const {
  return fromId<=0;
}

bool Connection::danglingEnd() const {
  return toId<=0;
}

Connection::Connection() {
  id = IDFactory::instance().newId();
  fromId = toId = 0;
  fromPin = toPin = PinID::NOPIN;
  via = QPolygon();
}

Connection::Connection(PinID from1, PinID to1): Connection() {
  setFrom(from1);
  setTo(to1);
}


QXmlStreamReader &operator>>(QXmlStreamReader &sr, Connection &c) {
  auto a = sr.attributes();

  c = Connection();
  c.id = a.value("id").toInt();
  QStringList from = a.value("from").toString().split(":");
  if (from.size()>=1) 
    c.fromId = from.takeFirst().toInt();
  c.fromPin = from.join(":");
  QStringList to = a.value("to").toString().split(":");
  if (to.size()>=1) 
    c.toId = to.takeFirst().toInt();
  c.toPin = to.join(":");
  QStringList via = a.value("via").toString().split(" ");
  for (QString &v: via) {
    QStringList xy = v.split(",");
    if (xy.size()==2) 
      c.via << QPoint(xy[0].toInt(), xy[1].toInt());
  }

  sr.skipCurrentElement();
  return sr;
}
  
QXmlStreamWriter &operator<<(QXmlStreamWriter &sw, Connection const &c) {
  sw.writeStartElement("connection");

  sw.writeAttribute("id", QString::number(c.id));
  sw.writeAttribute("from",
                    QString("%1:%2").arg(c.fromId).arg(c.fromPin));
  sw.writeAttribute("to",
                    QString("%1:%2").arg(c.toId).arg(c.toPin));
  QStringList v;
  for (QPoint const &p: c.via)
    v << QString("%1,%2").arg(p.x()).arg(p.y());
  if (!v.isEmpty())
    sw.writeAttribute("via", v.join(" "));

  sw.writeEndElement();
  return sw;
}

void Connection::unsetFrom() {
  setFrom(0, PinID::NOPIN);
}

void Connection::unsetTo() {
  setTo(0, PinID::NOPIN);
}

void Connection::setFrom(int id, QString pin) {
  fromId = id;
  fromPin = pin;
}

void Connection::setTo(int id, QString pin) {
  toId = id;
  toPin = pin;
}

Connection Connection::translated(QPoint delta) const {
  Connection c = *this;
  c.translate(delta);
  return c;
}

void Connection::translate(QPoint delta) {
  for (QPoint &p: via)
    p += delta;
}

Connection Connection::reversed() const {
  Connection con = *this;
  con.via.clear();
  for (auto p: via)
    con.via.prepend(p);
  con.setFrom(to());
  con.setTo(from());
  return con;
}

void Connection::reverse() {
  *this = reversed();
}

PinID Connection::from() const {
  return PinID(fromId, fromPin);
}

PinID Connection::to() const {
  return PinID(toId, toPin);
}

void Connection::setFrom(PinID pp) {
  fromId = pp.element();
  fromPin = pp.pin();
}

void Connection::setTo(PinID pp) {
  toId = pp.element();
  toPin = pp.pin();
}

bool Connection::isEquivalentTo(Connection const &o) const {
  return (from()==o.from() && to()==o.to())
    ||  (from()==o.to() && to()==o.from());
}

QString Connection::report() const {
  QString x = QString("c#%1: [%2:%3 ; ").arg(id).arg(fromId).arg(fromPin);
  for (auto v: via)
    x += QString("%1,%2 ").arg(v.x()).arg(v.y());
  x += QString("; %1:%2]").arg(toId).arg(toPin);
  return x;
}


QDebug &operator<<(QDebug &dbg, Connection const &con) {
  dbg << con.report();
  return dbg;
}
