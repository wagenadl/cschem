// Symbol.cpp

#include "Symbol.h"
#include <QSvgRenderer>
#include <QDebug>
#include <iostream>

static QMap<QString, QSharedPointer<QSvgRenderer> > &symbolRenderers() {
  static QMap<QString, QSharedPointer<QSvgRenderer> > rnd;
  return rnd;
}

//static QMap<QString, QByteArray> &symbolData() {
//  static QMap<QString, QByteArray> map;
//  return map;
//}

class SymbolData: public QSharedData {
public:
  SymbolData(): valid(false) { }
  void newshift();
  void ensureBBox();
  void writeSvg(QXmlStreamWriter &sw, bool withpins) const;
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

QPointF Symbol::svgOrigin() const {
  return d->pins[d->originId];
}

void SymbolData::newshift() {
  QStringList pp = pinIds.keys();
  if (pp.isEmpty())
    return;
  originId = pp.first();
  QPointF origin = pins[originId];
  shbbox = bbox.translated(-origin);
  for (QString p: pp)
    shpins[p] = pins[p] - origin;
}

void Symbol::writeNamespaces(QXmlStreamWriter &sr) {
  sr.writeDefaultNamespace("http://www.w3.org/2000/svg");
  sr.writeNamespace("http://purl.org/dc/elements/1.1/", "dc");
  sr.writeNamespace("http://creativecommons.org/ns#", "cc");
  sr.writeNamespace("http://www.w3.org/1999/02/22-rdf-syntax-ns#", "rdf");
  sr.writeNamespace("http://www.w3.org/2000/svg", "svg");
  sr.writeNamespace("http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd",
                    "sodipodi");
  sr.writeNamespace("http://www.inkscape.org/namespaces/inkscape", "inkscape");
}

void SymbolData::writeSvg(QXmlStreamWriter &sw, bool withpins) const {
  static QSet<QString> skip{"pin","annotation"};
  elt.writeStartElement(sw);
  for (auto &c: elt.children()) {
    if (!withpins && c.type()==XmlNode::Type::Element
	&& skip.contains(c.element().attributes().value("inkscape:label")
			 .split(":").first().toString())) {
      // skip
    } else {
      c.write(sw);
    }
  }
  elt.writeEndElement(sw);
}

QByteArray SymbolData::toSvg(bool withbbox, bool withpins) const {
  QByteArray res;
  {
    QXmlStreamWriter sw(&res);
    sw.writeStartDocument("1.0", false);

    sw.writeStartElement("svg");
    sw.writeAttribute("version", "1.1");
    Symbol::writeNamespaces(sw);
    if (withbbox) {
      sw.writeAttribute("width", QString("%1").arg(bbox.width()));
      sw.writeAttribute("height", QString("%1").arg(bbox.height()));
      sw.writeAttribute("viewBox", QString("0 0 %1 %2")
			.arg(bbox.width()).arg(bbox.height()));
    }
    
    sw.writeStartElement("g");
    if (withbbox)
      sw.writeAttribute("transform",
			QString("translate(%1,%2)").
			arg(-bbox.left()).arg(-bbox.top()));
    
    writeSvg(sw, withpins);
    
    sw.writeEndElement(); // g [transform]
    sw.writeEndElement(); // svg
    sw.writeEndDocument();
  }
  return res;
}

void SymbolData::ensureBBox() {
  QByteArray svg = toSvg(false, true);
  QSvgRenderer renderer(svg);
  QString id = groupId;
  bbox = renderer.matrixForElement(id).mapRect(renderer.boundsOnElement(id))
    .toAlignedRect();
  qDebug() << id << bbox;
  for (QString pin: pins.keys())
    pins[pin]
      = renderer.matrixForElement(id)
      .map(renderer.boundsOnElement(pinIds[pin]).center());
  newshift();
}

Symbol::Symbol() {
  d = new SymbolData;
}

Symbol::Symbol(XmlElement const &elt): Symbol() {
  d->elt = elt;
  d->name = elt.attributes().value("inkscape:label").toString();
  for (auto &e: elt.children()) 
    if (e.type()==XmlNode::Type::Element)
      d->scanPins(e.element());
  d->ensureBBox();
  d->valid = true;
  forgetRenderer(*this);
}

Symbol::~Symbol() {
}

Symbol::Symbol(Symbol const &o) {
  d = o.d;
}

Symbol &Symbol::operator=(Symbol const &o) {
  d = o.d;
  return *this;
}

void SymbolData::scanPins(XmlElement const &elt) {
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

QStringList Symbol::pinNames() const {
  QStringList lst(d->pins.keys()); // QMap sorts its keys
  return lst;
}

QPointF Symbol::bbOrigin() const {
  QStringList l = d->pins.keys();
  return l.isEmpty() ? QPointF() : bbPinPosition(l.first());
}

QRectF Symbol::shiftedBBox() const {
  return d->shbbox;
}

QPointF Symbol::shiftedPinPosition(QString pinname) const {
  return d->shpins.contains(pinname)
    ? d->shpins[pinname]
    : QPointF();
}
    

QPointF Symbol::bbPinPosition(QString pinname) const {
  if (d->pins.contains(pinname))
    return d->pins[pinname] - d->bbox.topLeft();
  else
    return QPointF();
}

XmlElement const &Symbol::element() const {
  if (!isValid()) {
    qDebug() << "Caution: element() requested on invalid symbol";
  }
  return d->elt;
}

QString Symbol::name() const {
  return d->name;
}

bool Symbol::isValid() const {
  return d->valid;
}

QRectF Symbol::svgBBox() const {
  return d->bbox;
}

QByteArray Symbol::toSvg() const {
  return d->toSvg(true, false);
}

void Symbol::writeSvg(QXmlStreamWriter &sw) const {
  d->writeSvg(sw, false);
}

void Symbol::forgetRenderer(Symbol const &p) {
  qDebug() << "forgetrenderer" << p.name();
  symbolRenderers().remove(p.name());
}

QSharedPointer<QSvgRenderer> Symbol::renderer() const {
  auto &map = symbolRenderers();
  //  auto &data = symbolData();
  if (!map.contains(d->name)) 
    map[d->name] = QSharedPointer<QSvgRenderer>(new QSvgRenderer(toSvg()));
  return map[d->name];
}

  
