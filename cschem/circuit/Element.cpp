// Element.cpp

#include "Element.h"
#include "IDFactory.h"
#include <QPoint>
#include "PartNumbering.h"
#include <QRegularExpression>

Element::Element() {
  id = IDFactory::instance().newId();
  type = Element::Type::Invalid;
  flipped = false;
  valueVisible = nameVisible = false;
}

Element Element::junction(QPoint p) {
  Element elt;
  elt.type = Type::Junction;
  elt.position = p;
  elt.autoSetVisibility();
  return elt;
}

Element Element::port(QString subtype, QPoint p) {
  Element elt;
  elt.type = Type::Port;
  elt.subtype = subtype;
  elt.position = p;
  elt.autoSetVisibility();
  return elt;
}

Element Element::component(QString subtype, QPoint p) {
  Element elt;
  elt.type = Type::Component;
  elt.subtype = subtype;
  elt.position = p;
  elt.autoSetVisibility();
  return elt;
}

Element Element::translated(QPoint delta) const {
  Element e = *this;
  e.translate(delta);
  return e;
}

void Element::translate(QPoint delta) {
  position += delta;
}

bool Element::isValid() const {
  return type != Type::Invalid;
}

QString Element::tag() const {
  switch (type) {
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
  switch (type) {
  case Type::Invalid:
    return "";
  case Type::Component:
    return "part:" + subtype;
  case Type::Port:
    return "port:" + subtype;
  case Type::Junction:
    return "junction";
  }
  return "";
}

QXmlStreamReader &operator>>(QXmlStreamReader &sr, Element &c) {
  c = Element();
  QStringRef name = sr.name();
  if (name=="component")
    c.type = Element::Type::Component;
  else if (name=="port")
    c.type = Element::Type::Port;
  else if (name=="junction")
    c.type = Element::Type::Junction;
  else
    c.type = Element::Type::Invalid;

  auto a = sr.attributes();
  
  c.id = a.value("id").toInt();
  c.position = QPoint(a.value("x").toInt(), a.value("y").toInt());
  c.subtype = a.value("type").toString();
  c.value = a.value("value").toString();
  c.valuePosition = QPoint(a.value("valx").toInt(), a.value("valy").toInt());
  c.valueVisible = a.value("valvis").toInt() > 0;
  c.name = a.value("name").toString();
  c.namePosition = QPoint(a.value("namex").toInt(), a.value("namey").toInt());
  c.nameVisible = a.value("namevis").toInt() > 0;
  c.rotation = a.value("rotation").toInt();
  c.flipped = a.value("flip").toInt() ? true : false;
  c.notes = a.value("notes").toString();

  sr.skipCurrentElement();
  return sr;
}

QXmlStreamWriter &operator<<(QXmlStreamWriter &sw, Element const &c) {
  sw.writeStartElement(c.tag());

  sw.writeAttribute("id", QString::number(c.id));
  sw.writeAttribute("x", QString::number(c.position.x()));
  sw.writeAttribute("y", QString::number(c.position.y()));
  if (!c.subtype.isEmpty())
    sw.writeAttribute("type", c.subtype);
  if (!c.value.isEmpty()) {
    sw.writeAttribute("value", c.value);
    sw.writeAttribute("valx", QString::number(c.valuePosition.x()));
    sw.writeAttribute("valy", QString::number(c.valuePosition.y()));
    sw.writeAttribute("valvis", QString::number(c.valueVisible ? 1 : 0));
  }
  if (!c.name.isEmpty()) {
    sw.writeAttribute("name", c.name);
    sw.writeAttribute("namex", QString::number(c.namePosition.x()));
    sw.writeAttribute("namey", QString::number(c.namePosition.y()));
    sw.writeAttribute("namevis", QString::number(c.nameVisible ? 1 : 0));
  }
  
  if (!c.notes.isEmpty())
    sw.writeAttribute("notes", c.notes);
  
  if (c.rotation)
    sw.writeAttribute("rotation", QString::number(c.rotation));
  if (c.flipped)
    sw.writeAttribute("flip", "1");

  sw.writeEndElement();
  return sw;
};

QString Element::report() const {
  return QString("e#%1: [%2 at %3,%4 (%5) - %6 %7]")
    .arg(id).arg(symbol())
    .arg(position.x()).arg(position.y()).arg(rotation)
    .arg(value).arg(name);
}

void Element::autoSetVisibility() {
  nameVisible = PartNumbering::initiallyShowName(symbol());
  valueVisible = PartNumbering::initiallyShowValue(symbol());
}    

void Element::copyAnnotationsFrom(Element const &o) {
  notes = o.notes;
  value = o.value;
  name = o.name;
  valuePosition = o.valuePosition;
  namePosition = o.namePosition;
  valueVisible = o.valueVisible;
  nameVisible = o.nameVisible;
}

QDebug operator<<(QDebug dbg, Element const &elt) {
  dbg << elt.report();
  return dbg;
}

bool Element::isContainer() const {
  return symbol().split(":").contains("container");
}

bool Element::isNameWellFormed() const {
  return PartNumbering::isNameWellFormed(name);
}

QString Element::prefix() const {
  return PartNumbering::prefix(name);
}

int Element::number() const {
  return PartNumbering::number(name);
}

int Element::subNumber() const {
  return PartNumbering::subNumber(name);
}

QString Element::cname() const {
  return PartNumbering::cname(name);
}

QString Element::csuffix() const {
  return PartNumbering::csuffix(name);
}

bool Element::operator==(Element const &o) const {
  return type==o.type && position==o.position && subtype==o.subtype
    && value==o.value && name==o.name && notes==o.notes
    && id==o.id && rotation==o.rotation && flipped==o.flipped
    && valuePosition==o.valuePosition && namePosition==o.namePosition
    && valueVisible==o.valueVisible && nameVisible==o.nameVisible;
}

bool Element::operator!=(Element const &o) const {
  return !(*this == o);
}

QString Element::humanPinName(QString pin) const {
  QString en = name;
  switch (type) {
  case Element::Type::Invalid:
    return "invalid object";
  case Element::Type::Component: 
    if (en.isEmpty())
      en = "unnamed component";
    if (pin.isEmpty())
      return "pin of " + en;
    else if (pin==PinID::NOPIN)
      return en;
    else if (pin.toInt()>0)
      return "pin " + pin + " of " + en;
    else
      return QString("pin “%1” of %2").arg(pin).arg(en);
    
  case Element::Type::Port:
    if (en.isEmpty())
      en = QString("port “%1”").arg(symbol().split(":").last());
    else
      en = QString("port “%1”").arg(en);
    if (pin==PinID::NOPIN)
      return en;
    else
      return "pin of " + en;
  case Element::Type::Junction:
    return "junction";
  }
  return ""; // not executed
}
