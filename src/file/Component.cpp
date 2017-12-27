// Component.cpp

#include "Component.h"
#include "IDFactory.h"
#include <QPoint>

class ComponentData: public QSharedData {
public:
  ComponentData(): id(IDFactory::instance().newId()), rotation(0) { }
public:
  QPoint position;
  QString type;
  QString value;
  QString name;
  QString label;
  int id;
  int rotation;
};

Component::Component() {
  d = new ComponentData();
}

Component::Component(Component const &o) {
  d = o.d;
}

Component &Component::operator=(Component const &o) {
  d = o.d;
  return *this;
}

Component::~Component() {
}


Component::Component(QXmlStreamReader &src): Component() {
  auto a = src.attributes();
  d->position = QPoint(a.value("x").toInt(), a.value("y").toInt());
  d->type = a.value("type").toString();
  d->value = a.value("value").toString();
  d->name = a.value("name").toString();
  d->label = a.value("label").toString();
  d->id = a.value("id").toInt();
  d->rotation = a.value("rotation").toInt();
  src.skipCurrentElement();
}

QPoint Component::position() const {
  return d->position;
}

QString Component::type() const {
  return d->type;
}

QString Component::value() const {
  return d->value;
}

QString Component::name() const {
  return d->name;
}

QString Component::label() const {
  return d->label;
}

int Component::id() const {
  return d->id;
}
  
int Component::rotation() const {
  return d->rotation;
}

void Component::setPosition(QPoint p) {
  d->position = p;
}

void Component::setType(QString t) {
  d->type = t;
}

void Component::setValue(QString v) {
  d->value = v;
}

void Component::setName(QString n) {
  d->name = n;
}

void Component::setLabel(QString l) {
  d->label = l;
}

void Component::setId(int id) {
  d->id = id;
}

void Component::setRotation(int o) {
  d->rotation = o;
}

QXmlStreamReader &operator>>(QXmlStreamReader &sr, Component &c) {
  c = Component(sr);
  return sr;
}
  
QXmlStreamWriter &operator<<(QXmlStreamWriter &sr, Component const &c) {
  sr.writeStartElement("component");
  sr.writeAttribute("x", QString::number(c.position().x()));
  sr.writeAttribute("y", QString::number(c.position().y()));
  if (!c.type().isEmpty())
    sr.writeAttribute("type", c.type());
  if (!c.value().isEmpty())
    sr.writeAttribute("value", c.value());
  if (!c.name().isEmpty())
    sr.writeAttribute("name", c.name());
  if (!c.label().isEmpty())
    sr.writeAttribute("label", c.label());
  sr.writeAttribute("id", QString::number(c.id()));
  if (c.rotation())
    sr.writeAttribute("rotation", QString::number(c.rotation()));
  sr.writeEndElement();
  return sr;
};

