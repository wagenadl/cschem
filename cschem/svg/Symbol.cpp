// Symbol.cpp

#include "Symbol.h"
#include <QSvgRenderer>
#include <QDebug>
#include <QFileInfo>
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
  SymbolData(): valid(false), ncpins(0) { }
  void newshift();
  void ensureBBox();
  void writeSvg(QXmlStreamWriter &sw, bool withpins) const;
  QByteArray toSvg(bool withbbox, bool withpins) const;
  void scanPins(XmlElement const &elt);
  void setError(QString err);
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
  QMap<QString, QRectF> annotationBBox;
  QMap<QString, QRectF> shAnnotationBBox;
  QMap<int, QMap<QString, QString>> cpins;
  // maps subelement number to map of pin name to physical pin number
  int ncpins;
  QStringList errors;
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
  for (QString p: annotationBBox.keys())
    shAnnotationBBox[p] = annotationBBox[p].translated(-origin);
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
  static QSet<QString> skip{"pin", "annotation", "cp"};
  elt.writeStartElement(sw);
  for (auto &c: elt.children()) {
    if (!withpins && c.type()==XmlNode::Type::Element
	&& skip.contains(c.element().label().split(":").first())) {
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
  for (QString pin: pins.keys())
    pins[pin]
      = renderer.matrixForElement(id)
      .map(renderer.boundsOnElement(pinIds[pin]).center());
  for (QString ann: annotationBBox.keys())
    annotationBBox[ann]
      = renderer.matrixForElement(id)
      .mapRect(annotationBBox[ann]);
      
  newshift();
}

void SymbolData::setError(QString err) {
  valid = false;
  errors << err;
}

Symbol::Symbol() {
  d = new SymbolData;
  d->setError("Not a symbol file.");
}

Symbol::Symbol(XmlElement const &elt, QString name) {
  d = new SymbolData;
  d->elt = elt;
  d->elt.removeAttribute("transform");
  if (name.isEmpty()) {
    d->name = elt.label();
  } else {
    d->name = name;
    d->elt.setTitle(name);
  }
  if (d->name.isEmpty())
    d->setError("Symbol has no name.");
  else
    d->valid = true;
  for (auto &e: elt.children()) 
    if (e.type()==XmlNode::Type::Element)
      d->scanPins(e.element());
  if (d->valid && d->pins.isEmpty())
    d->setError("No pins found in symbol definition.");
  d->ensureBBox();
  forgetRenderer(*this);
  // qDebug() << stats();
}

Symbol::~Symbol() {
}

Symbol Symbol::load(QString svgfn) {
  Symbol sym;
  QFile file(svgfn);
  QString name = QFileInfo(svgfn).baseName();
  name.replace("-", ":");
  if (!name.startsWith("port:") && !name.startsWith("part:"))
    name = "part:" + name;
  //qDebug() << "symbol load" << svgfn << " named " << name
  //         << " exists " << file.exists();
  if (!file.open(QFile::ReadOnly)) 
    return sym;
  QXmlStreamReader sr(&file);
  int groupcount = 0;
  while (!sr.atEnd()) {
    sr.readNext();
    if (sr.isStartElement() && sr.qualifiedName()=="svg") {
      XmlElement svg(sr);
      for (auto &c: svg.children()) {
        if (c.type()==XmlNode::Type::Element) {
          XmlElement elt(c.element());
          if (elt.qualifiedName()=="g") {
            if (groupcount==0)
              sym = Symbol(elt, name);
            groupcount++;
          }
        }
      }
    }
  }
  //qDebug() << "Loaded symbol";
  //for (QString s: sym.stats().split("\n"))
  //  qDebug() << s;
  //qDebug() << "--";
  if (groupcount>1)
    qDebug() << "Only the first group was read";
  
  return sym;
}

Symbol::Symbol(Symbol const &o) {
  d = o.d;
}

Symbol &Symbol::operator=(Symbol const &o) {
  d = o.d;
  return *this;
}

void SymbolData::scanPins(XmlElement const &elt) {
  /* Scans for circles with name matching {pin:NAME},
     If NAME matches {cp.NUM/SUB} it is considered a contained pin with number
     num and subname SUB. If SUB is "nc", that is considered a not-connected
     pin. Otherwise, SUB should be of the form {SLOT/NAME}.
     Also scans for rectangles with name matching {annotation:WHAT} where
     WHAT must be "ref" (or "name") or "value".
   */
  //qDebug() << "scanpins" << elt.qualifiedName() << elt.title() << elt.label();
  if (elt.qualifiedName()=="circle") {
    QString label = elt.title();
    if (label.isEmpty())
      label = elt.label();
    if (label.startsWith("pin")) {
      QString name = label.mid(4);
      double x = elt.attributes().value("cx").toDouble();
      double y = elt.attributes().value("cy").toDouble();
      pins[name] = QPointF(x, y);
      pinIds[name] = elt.attributes().value("id").toString();
    } else if (label.startsWith("cp")) {
      QString name = label.mid(3);
      int sidx = name.indexOf("/");
      if (sidx>0 && name.mid(sidx+1)=="nc") {
	ncpins ++;
      } else {
	int didx = name.indexOf(".");
	if (sidx>0 && didx>sidx) {
	  int n = name.mid(sidx+1,didx-sidx-1).toInt();
	  if (n<=0) {
	    qDebug() << "Symbol: Nonpositive slot??";
          setError("All “CP” pins should have a positive slot number. “"
                   + name + "” is ill-formed.");
          }
	  QString sub = name.mid(didx+1);
	  cpins[n][sub] = name.left(sidx);
	} else {
          qDebug() << "Symbol: CP without slot number??";
          setError("All “CP” pins should have a slot number. “"
                   + name + "” is ill-formed.");
        }
      }
    }
  } else if (elt.qualifiedName()=="rect") {
    QString label = elt.title();
    if (label.isEmpty())
      label = elt.label();
    if (label.startsWith("annotation:")) {
      QStringList bits = label.split(":");
      if (bits.size()>=2) {
	QString name = bits[1];
        if (name=="ref")
          name = "name";
	double x = elt.attributes().value("x").toDouble();
	double y = elt.attributes().value("y").toDouble();
	double w = elt.attributes().value("width").toDouble();
	double h = elt.attributes().value("height").toDouble();
	annotationBBox[name] = QRectF(QPointF(x, y), QSizeF(w, h));
      }
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


QByteArray Symbol::toSvg() const {
  return d->toSvg(true, false);
}

void Symbol::writeSvg(QXmlStreamWriter &sw) const {
  d->writeSvg(sw, false);
}

void Symbol::forgetRenderer(Symbol const &p) {
  symbolRenderers().remove(p.name());
}

QSharedPointer<QSvgRenderer> Symbol::renderer() const {
  auto &map = symbolRenderers();
  //  auto &data = symbolData();
  if (!map.contains(d->name)) 
    map[d->name] = QSharedPointer<QSvgRenderer>(new QSvgRenderer(toSvg()));
  return map[d->name];
}

  
QRectF Symbol::shiftedAnnotationBBox(QString id) const {
  return d->shAnnotationBBox.contains(id)
    ? d->shAnnotationBBox[id]
    : QRectF();
}

QString Symbol::prefixForSlotCount(int sc) {
  switch (sc) {
  case 2: return "½ ";
  case 3: return "⅓ ";
  case 4: return "¼ ";
  case 6: return "⅙ ";
  case 8: return "⅛ ";
  }
  return "";
}

QList<int> Symbol::containerSlots() const {
  return d->cpins.keys();
}

int Symbol::slotCount() const {
  int sc = containerSlots().count();
  return sc > 1 ? sc : 1;
}

QMap<QString, QString> Symbol::containedPins(int slot) const {
  if (d->cpins.contains(slot))
    return d->cpins[slot];
  else
    return QMap<QString, QString>();
}

int Symbol::totalPinCount() const {
  int npins = pinNames().size();
  for (int n: containerSlots())
    npins += containedPins(n).size();
  npins += d->ncpins;
  return npins;
}

QString Symbol::stats() const {
  QStringList res;
  res << "Symbol name: " + d->name;
  res << "Pin count: " + QString::number(d->pins.size());
  res << "Pins:";
  for (auto it=d->pinIds.begin(); it!=d->pinIds.end(); ++it) 
    res << "  " + it.key() + ": " + it.value();
  res << "Unconnected pin count: " + QString::number(d->ncpins);
  res << "Container slot count: " + QString::number(slotCount());
  res << "Container slots:";
  for (auto it1=d->cpins.begin(); it1!=d->cpins.end(); ++it1) {
    res << "  " + QString::number(it1.key());
    for (auto it=it1.value().begin(); it!=it1.value().end(); ++it)
      res << "    " + it.key() + ": " + it.value();
  }
  return res.join("\n");
}

QStringList Symbol::problems() const {
  return d->errors;
}
