// Element.cpp

#include "Element.h"
#include "IDFactory.h"
#include <QPoint>

class ElementData: public QSharedData {
public:
  ElementData(): type(Element::Type::Invalid),
                 id(IDFactory::instance().newId()),
                 rotation(0) { }
public:
  Element::Type type;
  QPoint position;
  QString subtype;
  QString value;
  QString name;
  QPoint valuePos;
  QPoint namePos;
  bool valueVis;
  bool nameVis;
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
  d->valuePos = QPoint(a.value("valx").toInt(), a.value("valy").toInt());
  d->valueVis = a.value("valvis").toInt() > 0;
  d->name = a.value("name").toString();
  d->namePos = QPoint(a.value("namex").toInt(), a.value("namey").toInt());
  d->nameVis = a.value("namevis").toInt() > 0;
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

QPoint Element::valuePos() const {
  return d->valuePos;
}

bool Element::isValueVisible() const {
  return d->valueVis;
}

QString Element::name() const {
  return d->name;
}

QPoint Element::namePos() const {
  return d->namePos;
}

bool Element::isNameVisible() const {
  return d->nameVis;
}

int Element::id() const {
  return d->id;
}
  
int Element::rotation() const {
  return d->rotation;
}

void Element::setPosition(QPoint p) {
  d.detach();
  d->position = p;
}

Element Element::translated(QPoint delta) const {
  Element e = *this;
  e.translate(delta);
  return e;
}

void Element::translate(QPoint delta) {
  d.detach();
  d->position += delta;
}

void Element::setSubtype(QString t) {
  d.detach();
  d->subtype = t;
}

void Element::setValue(QString v) {
  d.detach();
  d->value = v;
}

void Element::setValuePos(QPoint p) {
  d.detach();
  d->valuePos = p;
}

void Element::setValueVisible(bool b) {
  d.detach();
  d->valueVis = b;
}

void Element::setName(QString n) {
  d.detach();
  d->name = n;
}

void Element::setNamePos(QPoint p) {
  d.detach();
  d->namePos = p;
}

void Element::setNameVisible(bool b) {
  d.detach();
  d->nameVis = b;
}

void Element::setId(int id) {
  d.detach();
  d->id = id;
}

void Element::setRotation(int o) {
  d.detach();
  d->rotation = o & 3;
}

bool Element::isValid() const {
  return d->type != Type::Invalid;
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
  if (!c.value().isEmpty()) {
    sr.writeAttribute("value" , c.value());
    sr.writeAttribute("valx", QString::number(c.valuePos().x()));
    sr.writeAttribute("valy", QString::number(c.valuePos().y()));
    sr.writeAttribute("valvis", QString::number(c.isValueVisible() ? 1 : 0));
  }
  if (!c.name().isEmpty()) {
    sr.writeAttribute("name", c.name());
    sr.writeAttribute("namex", QString::number(c.namePos().x()));
    sr.writeAttribute("namey", QString::number(c.namePos().y()));
    sr.writeAttribute("namevis", QString::number(c.isNameVisible() ? 1 : 0));
  }
  
  if (c.rotation())
    sr.writeAttribute("rotation", QString::number(c.rotation()));
  sr.writeEndElement();
  return sr;
};

QString Element::report() const {
  return QString("%1: %2 at %3,%4 (%5) - %6 %7")
    .arg(id()).arg(symbol())
    .arg(position().x()).arg(position().y()).arg(rotation())
    .arg(value()).arg(name());
}
    
