// Part.cpp

#include "Part.h"

class PartData: public QSharedData {
public:
  PartData(): valid(false) { }
public:
  XmlElement elt;
  QString name;
  QMap<QString, QPointF> pins;
  bool valid;
  QRectF bbox;
  QString groupId;
  QMap<QString, QString> pinIds;
};

Part::Part() {
  d = new PartData;
}

Part::Part(XmlElement const &elt): Part() {
  d->elt = elt;
  d->name = elt.attributes().value("inkscape:label").toString();
  for (auto &e: elt.children()) 
    if (e.type()==XmlNode::Type::Element)
      scanPins(e.element());

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

QPointF Part::origin() const {
  QStringList l = d->pins.keys();
  return l.isEmpty() ? QPointF() : pinPosition(l.first());
}

QRectF Part::shiftedBBox() const {
  QStringList l = d->pins.keys();
  return d->bbox.translated(l.isEmpty() ? -d->bbox.topLeft()
			    : - d->pins[l.first()]);
}

QPointF Part::pinPosition(QString pinname) const {
  if (d->pins.contains(pinname))
    return d->pins[pinname] - d->bbox.topLeft();
  else
    return QPointF();
}

void Part::setAbsPinPosition(QString pinname, QPointF pos) {
  if (d->pins.contains(pinname))
    d->pins[pinname] = pos;
}

void Part::setBBox(QRectF b) {
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

QRectF Part::bbox() const {
  return d->bbox;
}
