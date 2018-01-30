// SvgExporter.cpp

#include "SvgExporter.h"
#include "Geometry.h"
#include "SymbolLibrary.h"
#include "Symbol.h"
#include "circuit/Circuit.h"
#include "circuit/Element.h"
#include "circuit/Connection.h"
#include "XmlElement.h"
#include <QXmlStreamWriter>
#include <QDebug>
#include "ui/Style.h"
#include <QTransform>
#include "circuit/Schem.h"

class SvgExporterData {
public:
  SvgExporterData(Schem const &schem):
    circ(schem.circuit()), lib(schem.library()),
    geom(circ, lib) {
  }
  void writeBBox(QXmlStreamWriter &sw);
  void writeElement(QXmlStreamWriter &sw, Element const &elt);
  void writeConnection(QXmlStreamWriter &sw, Connection const &elt);
public:
  Circuit circ;
  SymbolLibrary const &lib;
  Geometry geom;
};


void SvgExporterData::writeElement(QXmlStreamWriter &sw, Element const &elt) {
  QString sym = elt.symbol();
  Symbol const &symbol = lib.symbol(sym);

  QPointF p0 = lib.upscale(elt.position);
  int r = elt.rotation;
  bool flp = elt.flipped;

  sw.writeStartElement("g");
  QString xform = QString("translate(%1,%2)").arg(p0.x()).arg(p0.y());
  sw.writeAttribute("transform", xform);
  xform = "";
  if (r)
    xform += QString(" rotate(%1)").arg(r*-90);
  if (flp)
    xform += QString(" scale(-1,1)");
  QPointF p1 = symbol.svgOrigin();
  xform += QString(" translate(%1,%2)").arg(-p1.x()).arg(-p1.y());
  sw.writeStartElement("g");
  sw.writeAttribute("transform", xform);
  qDebug() << "writing " << sym << symbol.isValid();
  symbol.writeSvg(sw);
  sw.writeEndElement(); // g

  // origin for labels
  QPointF po = symbol.shiftedBBox().center();
  QTransform xf;
  xf.rotate(elt.rotation*-90);
  if (elt.flipped)
    xf.scale(-1, 1);
  po = xf.map(po);
  
  if (elt.nameVisible) {
    QPointF p = elt.namePosition + po;
    sw.writeStartElement("text");
    sw.writeAttribute("style", "fill:#000000;stroke:none;font-style:italic");
    sw.writeAttribute("x", QString("%1").arg(p.x()));
    sw.writeAttribute("y", QString("%1").arg(p.y()));
    QString txt = elt.name;
    if (txt.mid(1).toDouble()>0) {
      // letter + number
      sw.writeCharacters(txt.left(1));
      sw.writeStartElement("tspan");
      sw.writeAttribute("dy", "5");
      sw.writeAttribute("style", QString("font-style:normal;font-size:%1")
			.arg(Style::annotationFont().pointSize()*2));
      sw.writeCharacters(txt.mid(1));
      sw.writeEndElement(); // tspan
    } else {
      sw.writeCharacters(txt);
    }
    sw.writeEndElement();
  }
  if (elt.valueVisible) {
    QPointF p = elt.valuePosition + po;
    sw.writeStartElement("text");
    sw.writeAttribute("style", "fill:#000000;stroke:none");
    sw.writeAttribute("x", QString("%1").arg(p.x()));
    sw.writeAttribute("y", QString("%1").arg(p.y()));
    QString txt = elt.value;
    sw.writeCharacters(txt);
    sw.writeEndElement();
  }
  sw.writeEndElement(); // g
}

void SvgExporterData::writeConnection(QXmlStreamWriter &sw, Connection const &con) {
  QPolygon pp(geom.connectionPath(con));
  sw.writeStartElement("path");
  sw.writeAttribute("id", QString("con%1").arg(con.id));
  QStringList bits;
  QPointF p = lib.upscale(pp.takeFirst());
  bits << QString("M %1,%2").arg(p.x()).arg(p.y());
  for (QPoint p0: pp) {
    QPointF p = lib.upscale(p0);
    bits << QString("L %1,%2").arg(p.x()).arg(p.y());
  }
  sw.writeAttribute("d", bits.join(" "));
  sw.writeEndElement();
}

void SvgExporterData::writeBBox(QXmlStreamWriter &sw) {
  QRectF bbox = lib.upscale(geom.boundingRect());
  // Add space for annotations. This could be more elegant:
  double s = lib.scale();
  double mrg = 6*s;
  bbox.adjust(-mrg, -mrg, mrg, mrg);

  sw.writeAttribute("width", QString("%1").arg(bbox.width()));
  sw.writeAttribute("height", QString("%1").arg(bbox.height()));
  sw.writeAttribute("viewBox", QString("%1 %2 %3 %4")
		    .arg(bbox.left()).arg(bbox.top())
		    .arg(bbox.width()).arg(bbox.height()));
}

SvgExporter::SvgExporter(Schem const &schem):
  d(new SvgExporterData(schem)) {
}

SvgExporter::~SvgExporter() {
  delete d;
}

bool SvgExporter::exportSvg(QString const &fn) {
  qDebug() << "symbol names" << d->lib.symbolNames();
  QFile file(fn);
  if (file.open(QFile::WriteOnly)) {
    QXmlStreamWriter sw(&file);
    sw.setAutoFormatting(true);
    sw.setAutoFormattingIndent(2);
    sw.writeStartDocument("1.0", false);
    sw.writeStartElement("svg");
    Symbol::writeNamespaces(sw);
    d->writeBBox(sw);

    sw.writeStartElement("g"); // put the whole thing in a group
    QString style = "opacity:1;fill:none;fill-opacity:1;fill-rule:nonzero;";
    style += "stroke:#000000;";
    style += QString("stroke-width:%1;").arg(d->lib.lineWidth());
    style += "stroke-linecap:square;stroke-linejoin:miter;stroke-miterlimit:4;";
    style += "stroke-dasharray:none;stroke-dashoffset:0;stroke-opacity:1;";
    style += "font-style:normal;font-variant:normal;font-weight:normal;";
    style += QString("font-size:%1px;font-family:%2;")
      .arg(Style::annotationFont().pointSize()*2.62)
      .arg(Style::annotationFont().family());
    sw.writeAttribute("style", style);
    
    for (Element const &elt: d->circ.elements())
      d->writeElement(sw, elt);

    for (Connection const &con: d->circ.connections())
      d->writeConnection(sw, con);
    sw.writeEndElement(); // g
    sw.writeEndElement(); // svg
    sw.writeEndDocument();
  } else {
    qDebug() << "Failed to export" << fn;
    return false;
  }
  return true;
}

