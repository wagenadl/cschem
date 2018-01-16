// Element.cpp

#include "Element.h"
#include "IDFactory.h"
#include <QPoint>

class ElementData: public QSharedData {
public:
  ElementData(): id(IDFactory::instance().newId()) {
    reset();
  }
  void reset() {
    type = Element::Type::Invalid;
    layer = Layer::Schematic;
    position = QPoint();
    subtype = "";
    value = name = "";
    rotation = 0;
    flip = false;
    info = Element::Info();
    valuePos = namePos = QPoint();
    valueVis = nameVis = false;
  }
public:
  Element::Type type;
  Layer layer;
  QPoint position;
  QString subtype;
  QString value;
  QString name;
  int id;
  int rotation;
  bool flip;
  Element::Info info;
  QPoint valuePos;
  QPoint namePos;
  bool valueVis;
  bool nameVis;
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

void Element::readAttributes(QXmlStreamReader &src) {
  auto a = src.attributes();
  d->id = a.value("id").toInt();
  d->position = QPoint(a.value("x").toInt(), a.value("y").toInt());
  d->layer = layerFromAbbreviation(a.value("layer").toString());
  d->subtype = a.value("type").toString();
  d->value = a.value("value").toString();
  d->valuePos = QPoint(a.value("valx").toInt(), a.value("valy").toInt());
  d->valueVis = a.value("valvis").toInt() > 0;
  d->name = a.value("name").toString();
  d->namePos = QPoint(a.value("namex").toInt(), a.value("namey").toInt());
  d->nameVis = a.value("namevis").toInt() > 0;
  d->rotation = a.value("rotation").toInt();
  d->flip = a.value("flip").toInt() ? true : false;
  d->info.vendor = a.value("vendor").toString();
  d->info.partno = a.value("partno").toString();
  d->info.notes = a.value("notes").toString();
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

bool Element::isFlipped() const {
  return d->flip;
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

void Element::setFlipped(bool f) {
  d.detach();
  d->flip = f;
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
  case Type::Via:
    return "via";
  case Type::Hole:
    return "hole";
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
  case Type::Via: case Type::Hole:
    return "hole:" + subtype();
  }
  return "";
}

QXmlStreamReader &operator>>(QXmlStreamReader &sr, Element &c) {
  c.d->reset();
  QStringRef name = sr.name();
  if (name=="component")
    c.d->type = Element::Type::Component;
  else if (name=="port")
    c.d->type = Element::Type::Port;
  else if (name=="junction")
    c.d->type = Element::Type::Junction;
  else if (name=="via")
    c.d->type = Element::Type::Via;
  else if (name=="hole")
    c.d->type = Element::Type::Hole;
  else
    c.d->type = Element::Type::Invalid;
  c.readAttributes(sr);
  sr.skipCurrentElement();
  return sr;
}

QXmlStreamWriter &operator<<(QXmlStreamWriter &sw, Element const &c) {
  sw.writeStartElement(c.tag());
  c.writeAttributes(sw);
  sw.writeEndElement();
  return sw;
};

void Element::writeAttributes(QXmlStreamWriter &sw) const {
  sw.writeAttribute("id", QString::number(id()));
  sw.writeAttribute("x", QString::number(position().x()));
  sw.writeAttribute("y", QString::number(position().y()));
  if (!subtype().isEmpty())
    sw.writeAttribute("type", subtype());
  if (!value().isEmpty()) {
    sw.writeAttribute("value" , value());
    sw.writeAttribute("valx", QString::number(valuePos().x()));
    sw.writeAttribute("valy", QString::number(valuePos().y()));
    sw.writeAttribute("valvis", QString::number(isValueVisible() ? 1 : 0));
  }
  if (!name().isEmpty()) {
    sw.writeAttribute("name", name());
    sw.writeAttribute("namex", QString::number(namePos().x()));
    sw.writeAttribute("namey", QString::number(namePos().y()));
    sw.writeAttribute("namevis", QString::number(isNameVisible() ? 1 : 0));
  }
  
  QString l = layerToAbbreviation(layer());
  if (!l.isEmpty())
    sw.writeAttribute("layer", l);
  
  if (!info().vendor.isEmpty())
    sw.writeAttribute("vendor", info().vendor);
  if (!info().partno.isEmpty())
    sw.writeAttribute("partno", info().partno);
  if (!info().notes.isEmpty())
    sw.writeAttribute("notes", info().notes);
  
  if (rotation())
    sw.writeAttribute("rotation", QString::number(rotation()));
  if (isFlipped())
    sw.writeAttribute("flip", "1");
}  

QString Element::report() const {
  return QString("%1: %2 at %3,%4 (%5) - %6 %7")
    .arg(id()).arg(symbol())
    .arg(position().x()).arg(position().y()).arg(rotation())
    .arg(value()).arg(name());
}
    
Element::Info Element::info() const {
  return d->info;
}

void Element::setInfo(Element::Info const &info) {
  d->info = info;
}

Layer Element::layer() const {
  return d->layer;
}

void Element::setLayer(Layer l) {
  d->layer = l;
}
