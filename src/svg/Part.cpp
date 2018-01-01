// Part.cpp

#include "Part.h"

class PartData: public QSharedData {
public:
  PartData(): valid(false) { }
  void newshift();
public:
  XmlElement elt;
  QString name;
  QMap<QString, QPointF> pins; // svg coordinates
  QMap<QString, QPointF> shpins; // shifted coords (origin=first pin)
  bool valid;
  QRectF bbox; // svg coords
  QRectF shbbox; // shifted coords (origin=first pin)
  QString groupId;
  QMap<QString, QString> pinIds;
  QString originId;
};

void PartData::newshift() {
  QStringList pp = pinIds.keys();
  if (pp.isEmpty())
    return;
  originId = pp.first();
  QPointF origin = pins[originId];
  shbbox = bbox.translated(-origin);
  for (QString p: pp)
    shpins[p] = pins[p] - origin;
}

Part::Part() {
  d = new PartData;
}

Part::Part(XmlElement const &elt): Part() {
  d->elt = elt;
  d->name = elt.attributes().value("inkscape:label").toString();
  for (auto &e: elt.children()) 
    if (e.type()==XmlNode::Type::Element)
      scanPins(e.element());
  d->newshift();
  d->valid = true;
}

Part::~Part() {
}

Part::Part(Part const &o) {
  d = o.d;
}

Part &Part::operator=(Part const &o) {
  d = o.d;
  return *this;
}

void Part::scanPins(XmlElement const &elt) {
  if (elt.qualifiedName()=="circle") {
    QString label = elt.attributes().value("inkscape:label").toString();
    if (label.startsWith("pin")) {
      QString name = label.mid(4);
      QString x = elt.attributes().value("cx").toString();
      QString y = elt.attributes().value("cy").toString();
      d->pins[name] = QPointF(x.toInt(), y.toInt());
      d->pinIds[name] = elt.attributes().value("id").toString();
    }
  } else if (elt.qualifiedName()=="g") {
    d->groupId = elt.attributes().value("id").toString();
  }
}

QStringList Part::pinNames() const {
  QStringList lst(d->pins.keys()); // QMap sorts its keys
  return lst;
}

QString Part::pinSvgId(QString pinname) const {
  if (d->pinIds.contains(pinname))
    return d->pinIds[pinname];
  else
    return "";
}

QString Part::contentsSvgId() const {
  return d->groupId;
}

QPointF Part::bbOrigin() const {
  QStringList l = d->pins.keys();
  return l.isEmpty() ? QPointF() : bbPinPosition(l.first());
}

QRectF Part::shiftedBBox() const {
  return d->shbbox;
}

QPointF Part::shiftedPinPosition(QString pinname) const {
  return d->shpins.contains(pinname)
    ? d->shpins[pinname]
    : QPointF();
}
    

QPointF Part::bbPinPosition(QString pinname) const {
  if (d->pins.contains(pinname))
    return d->pins[pinname] - d->bbox.topLeft();
  else
    return QPointF();
}

void Part::setSvgPinPosition(QString pinname, QPointF pos) {
  if (!d->pins.contains(pinname))
    return;
  d->pins[pinname] = pos;
  if (pinname == d->originId)
    d->newshift();
}

void Part::setSvgBBox(QRectF b) {
  d->bbox = b;
}

XmlElement const &Part::element() const {
  return d->elt;
}

QString Part::name() const {
  return d->name;
}

bool Part::isValid() const {
  return d->valid;
}

QRectF Part::svgBBox() const {
  return d->bbox;
}

