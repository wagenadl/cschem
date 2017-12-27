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

Part::Part() {
  d = new PartData;
}

Part::Part(XmlElement const &elt): Part() {
  d->elt = elt;
  d->name = elt.attributes().value("inkscape:label").toString();
  scanPins(elt);
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
      d->pins[name] = QPoint(x.toInt(), y.toInt());
    }
  }
  for (auto &e: elt.children()) 
    if (e.type()==XmlNode::Type::Element)
      scanPins(e.element());
}

QStringList Part::pinNames() const {
  QStringList lst(d->pins.keys()); // QMap sorts its keys
  return lst;
}

QPoint Part::origin() const {
  QStringList l = d->pins.keys();
  return l.isEmpty() ? QPoint() : pinPosition(l.first());
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

bool Part::isValid() const {
  return d->valid;
}

QRect Part::bbox() const {
  return d->bbox;
}
