// Part.cpp

#include "Part.h"
#include <QSvgRenderer>
#include <QDebug>

class PartData: public QSharedData {
public:
  PartData(): valid(false) { }
  void newshift();
  void ensureBBox();
  QByteArray toSvg(bool withbbox, bool withpins) const;
  void scanPins(XmlElement const &elt);
public:
  XmlElement elt;
  QString name;
  QMap<QString, QPointF> pins; // svg coordinates
  QMap<QString, QPointF> shpins; // shifted coords (origin=first pin)
  bool valid;
  QRectF bbox; // svg coords
  QRectF shbbox; // shifted coords (origin=first pin)
  QString groupId; // id of contents element
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

QByteArray PartData::toSvg(bool withbbox, bool withpins) const {
  QByteArray res;
  {
    QXmlStreamWriter sr(&res);
    sr.writeStartDocument("1.0", false);
    sr.writeStartElement("svg");
    sr.writeDefaultNamespace("http://www.w3.org/2000/svg");
    sr.writeNamespace("http://purl.org/dc/elements/1.1/", "dc");
    sr.writeNamespace("http://creativecommons.org/ns#", "cc");
    sr.writeNamespace("http://www.w3.org/1999/02/22-rdf-syntax-ns#", "rdf");
    sr.writeNamespace("http://www.w3.org/2000/svg", "svg");
    sr.writeNamespace("http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd",
                      "sodipodi");
    sr.writeNamespace("http://www.inkscape.org/namespaces/inkscape", "inkscape");
    
    if (withbbox) {
      sr.writeAttribute("width", QString("%1").arg(bbox.width()));
      sr.writeAttribute("height", QString("%1").arg(bbox.height()));
      sr.writeAttribute("viewBox", QString("0 0 %1 %2")
                        .arg(bbox.width()).arg(bbox.height()));
    }
    
    sr.writeStartElement("g");
    if (withbbox) 
      sr.writeAttribute("transform",
                        QString("translate(%1,%2)").
                        arg(-bbox.left()).arg(-bbox.top()));
    
    elt.writeStartElement(sr);
    for (auto &c: elt.children()) {
      if (!withpins && c.type()==XmlNode::Type::Element
          && c.element().attributes().value("inkscape:label")
          .startsWith("pin")) {
        // skip
      } else {
        c.write(sr);
      }
    }
    elt.writeEndElement(sr);
    
    sr.writeEndElement(); // g [transform]
    sr.writeEndElement(); // svg
    sr.writeEndDocument();
  }
  return res;
}


void PartData::ensureBBox() {
  QByteArray svg = toSvg(false, true);
  QSvgRenderer renderer(svg);
  QString id = groupId;
  bbox = renderer.boundsOnElement(id).toAlignedRect();
  for (QString pin: pins.keys())
    pins[pin] = renderer.boundsOnElement(pinIds[pin]).center();
  newshift();
}

Part::Part() {
  d = new PartData;
}

Part::Part(XmlElement const &elt): Part() {
  d->elt = elt;
  d->name = elt.attributes().value("inkscape:label").toString();
  for (auto &e: elt.children()) 
    if (e.type()==XmlNode::Type::Element)
      d->scanPins(e.element());
  d->ensureBBox();
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

void PartData::scanPins(XmlElement const &elt) {
  if (elt.qualifiedName()=="circle") {
    QString label = elt.attributes().value("inkscape:label").toString();
    if (label.startsWith("pin")) {
      QString name = label.mid(4);
      QString x = elt.attributes().value("cx").toString();
      QString y = elt.attributes().value("cy").toString();
      pins[name] = QPointF(x.toInt(), y.toInt());
      pinIds[name] = elt.attributes().value("id").toString();
    }
  } else if (elt.qualifiedName()=="g") {
    groupId = elt.attributes().value("id").toString();
  }
}

QStringList Part::pinNames() const {
  QStringList lst(d->pins.keys()); // QMap sorts its keys
  return lst;
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

QByteArray Part::toSvg() const {
  return d->toSvg(true, false);
}
