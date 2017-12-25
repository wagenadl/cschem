// Part.cpp

#include "Part.h"

class PartData: public QSharedData {
public:
  PartData(): valid(false) { }
public:
  XmlElement elt;
  QString name;
  QMap<QString, QPoint> pins;
  bool valid;
  QRect bbox;
};
  
Part::Part(XmlElement const &elt) {
  d = new PartData();
  d->elt = elt;
  d->name = elt.attributes().value("inkscape:label").toString();
  scanPins(elt);
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
      d->pins[name] = QPoint(x.toInt(), y.toInt());
    }
  }
  for (auto &e: elt.children()) 
    if (e.type()==XmlNode::Type::Element)
      scanPins(e.element());
}

QPoint Part::pinPosition(QString pinname) const {
  if (d->pins.contains(pinname))
    return d->pins[pinname] - d->bbox.topLeft();
  else
    return QPoint();
}

void Part::setBBox(QRect b) {
  d->bbox = b;
}

XmlElement const &Part::element() const {
  return d->elt;
}

QString Part::name() const {
  return d->name;
}

QMap<QString, QPoint> const &Part::pins() const {
  return d->pins;
}

bool Part::isValid() const {
  return d->valid;
}

QRect Part::bbox() const {
  return d->bbox;
}
