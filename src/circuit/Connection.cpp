// Connection.cpp

#include "Connection.h"
#include "IDFactory.h"
#include <QPoint>
#include <QDebug>

class ConnectionData: public QSharedData {
public:
  ConnectionData(): id(IDFactory::instance().newId()) {
    reset();
  }
  void reset() {
    fromId = toId = 0;
    fromPin = toPin = "";
    via = QPolygon();
    layer = Layer::Schematic;
    width = 0;
  }  
public:
  int id;
  int fromId;
  int toId;
  QString fromPin;
  QString toPin;
  QPolygon via;
  Layer layer;
  int width;
};

bool Connection::isValid() const {
  if (d->fromId>0 && d->toId>0)
    return d->fromId!=d->toId || d->fromPin!=d->toPin;
  else if (d->fromId>0 || d->toId>0)
    return d->via.size() >= 1;
  else
    return d->via.size() >= 2 && d->via.first() != d->via.last();
}

bool Connection::isDangling() const {
  return danglingStart() || danglingEnd();
}

bool Connection::danglingStart() const {
  return d->fromId<=0;
}

bool Connection::danglingEnd() const {
  return d->toId<=0;
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

Connection::Connection(PinID from1, PinID to1): Connection() {
  setFrom(from1);
  setTo(to1);
}


QXmlStreamReader &operator>>(QXmlStreamReader &sr, Connection &c) {
  c.d->reset();
  c.readAttributes(sr);
  sr.skipCurrentElement();
  return sr;
}

void Connection::readAttributes(QXmlStreamReader &sr) {
  auto a = sr.attributes();
  d->id = a.value("id").toInt();
  QStringList from = a.value("from").toString().split(":");
  if (from.size()>=1) 
    d->fromId = from.takeFirst().toInt();
  d->fromPin = from.join(":");
  QStringList to = a.value("to").toString().split(":");
  if (to.size()>=1) 
    d->toId = to.takeFirst().toInt();
  d->toPin = to.join(":");
  QStringList via = a.value("via").toString().split(" ");
  for (QString &v: via) {
    QStringList xy = v.split(",");
    if (xy.size()==2) 
      d->via << QPoint(xy[0].toInt(), xy[1].toInt());
  }
  d->layer = layerFromAbbreviation(a.value("layer").toString());
}
  
QXmlStreamWriter &operator<<(QXmlStreamWriter &sw, Connection const &c) {
  sw.writeStartElement("connection");
  c.writeAttributes(sw);
  sw.writeEndElement();
  return sw;
};

void Connection::writeAttributes(QXmlStreamWriter &sw) const {
  sw.writeAttribute("id", QString::number(id()));
  sw.writeAttribute("from",
                    QString("%1:%2").arg(fromId()).arg(fromPin()));
  sw.writeAttribute("to",
                    QString("%1:%2").arg(toId()).arg(toPin()));
  if (!via().isEmpty()) {
    QStringList v;
    for (QPoint const &p: via())
      v << QString("%1,%2").arg(p.x()).arg(p.y());
    sw.writeAttribute("via", v.join(" "));
  }
  
  QString l = layerToAbbreviation(layer());
  if (!l.isEmpty())
    sw.writeAttribute("layer", l);

  if (width()>0)
    sw.writeAttribute("width", QString("%1").arg(width()));
}


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

void Connection::unsetFrom() {
  setFrom(0, PinID::NOPIN);
}

void Connection::unsetTo() {
  setTo(0, PinID::NOPIN);
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

Layer Connection::layer() const {
  return d->layer;
}

int Connection::width() const {
  return d->width;
}
