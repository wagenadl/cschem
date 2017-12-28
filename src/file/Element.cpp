// Element.cpp

#include "Element.h"
#include "IDFactory.h"
#include <QPoint>

class ElementData: public QSharedData {
public:
  ElementData(): id(IDFactory::instance().newId()), rotation(0) { }
public:
  Element::Type type;
  QPoint position;
  QString subtype;
  QString value;
  QString name;
  QString label;
  int id;
  int rotation;
};

Element::Element() {
  d = new ElementData();
}

Element::Element(Element const &o) {
  d = o.d;
}

Element &Element::operator=(Element const &o) {
  d = o.d;
  return *this;
}

Element::~Element() {
}

Element::Element(QXmlStreamReader &src): Element() {
  auto a = src.attributes();
  QStringRef name = src.name();
  if (name=="component")
    d->type = Type::Component;
  else if (name=="port")
    d->type = Type::Port;
  else if (name=="junction")
    d->type = Type::Junction;
  else
    d->type = Type::Invalid;
  d->id = a.value("id").toInt();
  d->position = QPoint(a.value("x").toInt(), a.value("y").toInt());
  d->subtype = a.value("type").toString();
  d->value = a.value("value").toString();
  d->name = a.value("name").toString();
  d->label = a.value("label").toString();
  d->rotation = a.value("rotation").toInt();
  src.skipCurrentElement();
}

Element Element::junction(QPoint p) {
  Element elt;
  elt.d->type = Type::Junction;
  elt.d->position = p;
  return elt;
}

Element Element::port(QString subtype, QPoint p) {
  Element elt;
  elt.d->type = Type::Port;
  elt.d->subtype = subtype;
  elt.d->position = p;
  return elt;
}

Element Element::component(QString subtype, QPoint p) {
  Element elt;
  elt.d->type = Type::Component;
  elt.d->subtype = subtype;
  elt.d->position = p;
  return elt;
}

QPoint Element::position() const {
  return d->position;
}

QString Element::subtype() const {
  return d->subtype;
}

QString Element::value() const {
  return d->value;
}

QString Element::name() const {
  return d->name;
}

QString Element::label() const {
  return d->label;
}

int Element::id() const {
  return d->id;
}
  
int Element::rotation() const {
  return d->rotation;
}

void Element::setPosition(QPoint p) {
  d->position = p;
}

void Element::translate(QPoint delta) {
  d->position += delta;
}

void Element::setSubtype(QString t) {
  d->subtype = t;
}

void Element::setValue(QString v) {
  d->value = v;
}

void Element::setName(QString n) {
  d->name = n;
}

void Element::setLabel(QString l) {
  d->label = l;
}

void Element::setId(int id) {
  d->id = id;
}

void Element::setRotation(int o) {
  d->rotation = o;
}

Element::Type Element::type() const {
  return d->type;
}

QString Element::tag() const {
  switch (type()) {
  case Type::Invalid:
    return "";
  case Type::Component:
    return "component";
  case Type::Port:
    return "port";
  case Type::Junction:
    return "junction";
  }
  return "";
}

QString Element::symbol() const {
  switch (type()) {
  case Type::Invalid:
    return "";
  case Type::Component:
    return "part:" + subtype();
  case Type::Port:
    return "port:" + subtype();
  case Type::Junction:
    return "junction";
  }
  return "";
}

QXmlStreamReader &operator>>(QXmlStreamReader &sr, Element &c) {
  c = Element(sr);
  return sr;
}

QXmlStreamWriter &operator<<(QXmlStreamWriter &sr, Element const &c) {
  sr.writeStartElement(c.tag());
  sr.writeAttribute("id", QString::number(c.id()));
  sr.writeAttribute("x", QString::number(c.position().x()));
  sr.writeAttribute("y", QString::number(c.position().y()));
  if (!c.subtype().isEmpty())
    sr.writeAttribute("type", c.subtype());
  if (!c.value().isEmpty())
    sr.writeAttribute("value", c.value());
  if (!c.name().isEmpty())
    sr.writeAttribute("name", c.name());
  if (!c.label().isEmpty())
    sr.writeAttribute("label", c.label());
  if (c.rotation())
    sr.writeAttribute("rotation", QString::number(c.rotation()));
  sr.writeEndElement();
  return sr;
};

