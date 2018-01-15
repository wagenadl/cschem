// Part.cpp

#include "Part.h"
#include <QSvgRenderer>
#include <QDebug>

static QMap<QString, QSharedPointer<QSvgRenderer> > &partRenderers() {
  static QMap<QString, QSharedPointer<QSvgRenderer> > rnd;
  return rnd;
}

static QMap<QString, QByteArray> &partData() {
  static QMap<QString, QByteArray> map;
  return map;
}

class PartData: public QSharedData {
public:
  PartData(): valid(false) { }
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

QPointF Part::svgOrigin() const {
  return d->pins[d->originId];
}

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

void Part::writeNamespaces(QXmlStreamWriter &sr) {
  sr.writeDefaultNamespace("http://www.w3.org/2000/svg");
  sr.writeNamespace("http://purl.org/dc/elements/1.1/", "dc");
  sr.writeNamespace("http://creativecommons.org/ns#", "cc");
  sr.writeNamespace("http://www.w3.org/1999/02/22-rdf-syntax-ns#", "rdf");
  sr.writeNamespace("http://www.w3.org/2000/svg", "svg");
  sr.writeNamespace("http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd",
                    "sodipodi");
  sr.writeNamespace("http://www.inkscape.org/namespaces/inkscape", "inkscape");
}

void PartData::writeSvg(QXmlStreamWriter &sw, bool withpins) const {
  elt.writeStartElement(sw);
  for (auto &c: elt.children()) {
    if (!withpins && c.type()==XmlNode::Type::Element
	&& c.element().attributes().value("inkscape:label")
	.startsWith("pin")) {
      // skip
    } else {
      c.write(sw);
    }
  }
  elt.writeEndElement(sw);
}

QByteArray PartData::toSvg(bool withbbox, bool withpins) const {
  QByteArray res;
  {
    QXmlStreamWriter sw(&res);
    sw.writeStartDocument("1.0", false);

    sw.writeStartElement("svg");
    sw.writeAttribute("version", "1.1");
    sw.writeAttribute("id", "cschempart");
    Part::writeNamespaces(sw);
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

void PartData::ensureBBox() {
  QByteArray svg = toSvg(false, true);
  qDebug() << "ensureBBOX" << name;
  qDebug() << svg;
  qDebug() << "]]] ensureBBOX" << name;
  QSvgRenderer renderer(svg);
  QString id = groupId;
  bbox = renderer.boundsOnElement(id).toAlignedRect();
  qDebug() << id << bbox;
  for (QString pin: pins.keys())
    pins[pin] = renderer.boundsOnElement(pinIds[pin]).center();
  for (QString pin: pins.keys())
    qDebug() << "pin" << pin << pins[pin];
  newshift();
  for (QString pin: pins.keys())
    qDebug() << "shifted pin" << pin << shpins[pin];
  qDebug() << "finalsvg" << name;
  qDebug() << toSvg(true, false);
  qDebug() << "]]] finalsvg" << name;
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
  forgetRenderer(*this);
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
  if (!isValid()) {
    qDebug() << "Caution: element() requested on invalid part";
  }
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

void Part::writeSvg(QXmlStreamWriter &sw) const {
  d->writeSvg(sw, false);
}

void Part::forgetRenderer(Part const &p) {
  qDebug() << "forgetrenderer" << p.name();
  partRenderers().remove(p.name());
}

QSharedPointer<QSvgRenderer> Part::renderer() const {
  auto &map = partRenderers();
  auto &data = partData();
  if (!map.contains(d->name)) {
    data[d->name] = toSvg();
    map[d->name] = QSharedPointer<QSvgRenderer>(new QSvgRenderer(data[d->name]));
  }
  return map[d->name];
}

  
