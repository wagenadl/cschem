// Port.cpp

#include "Port.h"
#include "IDFactory.h"
#include <QPoint>

class PortData: public QSharedData {
public:
  PortData(): id(IDFactory::instance().newId()), rotation(0) { }
public:
  QPoint position;
  QString type;
  QString name;
  int id;
  int rotation;
};

Port::Port() {
  d = new PortData();
}

Port::Port(Port const &o) {
  d = o.d;
}

Port &Port::operator=(Port const &o) {
  d = o.d;
  return *this;
}

Port::~Port() {
}


Port::Port(QXmlStreamReader &src): Port() {
  auto a = src.attributes();
  d->position = QPoint(a.value("x").toInt(), a.value("y").toInt());
  d->type = a.value("type").toString();
  d->name = a.value("name").toString();
  d->id = a.value("id").toInt();
  d->rotation = a.value("rotation").toInt();
  src.skipCurrentElement();
}

QPoint Port::position() const {
  return d->position;
}

QString Port::type() const {
  return d->type;
}

QString Port::name() const {
  return d->name;
}

int Port::id() const {
  return d->id;
}
  
int Port::rotation() const {
  return d->rotation;
}

void Port::setPosition(QPoint p) {
  d->position = p;
}

void Port::setType(QString t) {
  d->type = t;
}

void Port::setName(QString n) {
  d->name = n;
}

void Port::setId(int id) {
  d->id = id;
}

void Port::setRotation(int o) {
  d->rotation = o;
}

QXmlStreamReader &operator>>(QXmlStreamReader &sr, Port &c) {
  c = Port(sr);
  return sr;
}
  
QXmlStreamWriter &operator<<(QXmlStreamWriter &sr, Port const &c) {
  sr.writeStartElement("port");
  sr.writeAttribute("x", QString::number(c.position().x()));
  sr.writeAttribute("y", QString::number(c.position().y()));
  if (!c.type().isEmpty())
    sr.writeAttribute("type", c.type());
  if (!c.name().isEmpty())
    sr.writeAttribute("name", c.name());
  sr.writeAttribute("id", QString::number(c.id()));
  if (c.rotation())
    sr.writeAttribute("rotation", QString::number(c.rotation()));
  sr.writeEndElement();
  return sr;
};

