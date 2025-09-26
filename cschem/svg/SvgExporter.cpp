// SvgExporter.cpp

#include "SvgExporter.h"
#include "Geometry.h"
#include "SymbolLibrary.h"
#include "Symbol.h"
#include "circuit/Circuit.h"
#include "circuit/Element.h"
#include "circuit/Connection.h"
#include "circuit/Textual.h"
#include "XmlElement.h"
#include <QXmlStreamWriter>
#include <QDebug>
#include "ui/Style.h"
#include <QFile>
#include <QTransform>
#include "circuit/Schem.h"
#include "circuit/PartNumbering.h"
#include <QRegularExpression>

char const *SUBDISPLACE = "-3.5";
char const *SUBSUBDISPLACE = "-2.5";
char const *SUPDISPLACE = "3.5";

class SvgExporterData {
public:
  SvgExporterData(Schem const &schem):
    circ(schem.circuit()), lib(schem.library()),
    geom(circ, lib) {
    allnames = circ.allNames();
  }
  void writeBBox(QXmlStreamWriter &sw);
  void writeElement(QXmlStreamWriter &sw, Element const &elt);
  void writeConnection(QXmlStreamWriter &sw, Connection const &elt);
  void writeTextual(QXmlStreamWriter &sw, Textual const &txt);
  void writeTextualLine(QXmlStreamWriter &sw, QString const &line);
public:
  Circuit circ;
  SymbolLibrary const &lib;
  Geometry geom;
  QSet<QString> allnames;
};

double svgFontSize(bool script=false) {
  return Style::annotationFont().pixelSize()*(script ? .7 : 1)*.95;
}

QString svgFontStyle(bool italic=false, bool script=false, bool left=false) {
  return QString("fill:#000000;"
		 "stroke:none;"
		 "text-anchor:%4;"
		 "font-family:%1;"
		 "font-style:%2;"
		 "font-size:%3px")
    .arg(Style::annotationFont().family())
    .arg(italic ? "italic" : "normal")
    .arg(svgFontSize(script))
    .arg(left ? "left" : "middle");
}

QPointF svgFontDelta() {
  return QPointF(0, svgFontSize() * .3);
}

/*
void testCircle(QXmlStreamWriter &sw, QPointF p0) {
  // Use this to test positioning.
  // It places a tiny red circle at the given point.
  sw.writeStartElement("circle");
  sw.writeAttribute("style", "font-style:normal;font-variant:normal;font-weight:normal;opacity:1;fill:none;fill-opacity:1;fill-rule:nonzero;stroke:#ff0000;stroke-width:1.5;stroke-linecap:butt;stroke-linejoin:miter;stroke-miterlimit:4;stroke-dasharray:none;stroke-dashoffset:0;stroke-opacity:1");
  sw.writeAttribute("cx", QString::number(p0.x()));
  sw.writeAttribute("cy", QString::number(p0.y()));
  sw.writeAttribute("r", "2");
  sw.writeEndElement();
}
*/

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

  symbol.writeSvg(sw);
  sw.writeEndElement(); // g

  if (elt.nameVisible) {
    // copy styling from SceneElement::nameTextToWidget
    // and PartNumbering::nameTextToToHtml
    QString name = elt.name;
    int subidx = -1;
    if (name.startsWith("V") || name.startsWith("I")) {
      subidx = 1;
    } else if (PartNumbering::isNameWellFormed(name)) {
      subidx = PartNumbering::prefix(name).size();
    } else if (elt.type == Element::Type::Component) {
      subidx = 1;
      while (subidx < name.size() && name[subidx].isPunct())
        subidx ++;
    }
    bool useitalic = subidx > 0;
    QPointF p = elt.namePosition + svgFontDelta();
    sw.writeStartElement("text");
    sw.writeAttribute("style", svgFontStyle(useitalic, false));
    sw.writeAttribute("x", QString("%1").arg(p.x()));
    sw.writeAttribute("y", QString("%1").arg(p.y()));
    sw.writeCharacters(useitalic ? name.left(subidx) : name);
    if (useitalic) {
      sw.writeStartElement("tspan");
      sw.writeAttribute("baseline-shift", SUBDISPLACE);
      sw.writeAttribute("style", svgFontStyle(false, true));
      sw.writeCharacters(name.mid(subidx));
      sw.writeEndElement(); // tspan
    }
    sw.writeEndElement();
  }
  if (elt.valueVisible) {
    // copy styling from SceneElement::valueTextToWidget
    QPointF p = elt.valuePosition + svgFontDelta();
    QString txt = elt.value;
    sw.writeStartElement("text");
    sw.writeAttribute("style", svgFontStyle(false, false));
    sw.writeAttribute("x", QString("%1").arg(p.x()));
    sw.writeAttribute("y", QString("%1").arg(p.y()));
    if ((txt.startsWith("“V") || txt.startsWith("“I"))
	&& txt.endsWith("”")) {
      sw.writeCharacters("“");
      sw.writeStartElement("tspan");
      sw.writeAttribute("style", svgFontStyle(true, false));
      sw.writeCharacters(txt.mid(1, 1));
      sw.writeEndElement(); // tspan
      sw.writeStartElement("tspan");
      sw.writeAttribute("baseline-shift", SUBDISPLACE);
      sw.writeAttribute("style", svgFontStyle(false, true));
      sw.writeCharacters(txt.mid(2, txt.size()-3));
      sw.writeEndElement(); // tspan
      sw.writeCharacters("”");
    } else {
      sw.writeCharacters(txt);
    }
    sw.writeEndElement(); // text
  }
  sw.writeEndElement(); // g
}


void SvgExporterData::writeTextualLine(QXmlStreamWriter &sw, QString const &line) {
  /* Make sure logic is copied from SceneTextual::lineToHtml */
  QRegularExpression minus("(^|(?<=\\s))-($|(?=[\\s.0-9]))");
  QStringList bits;
  QString bit;
  bool inword = false;
  int len = line.length();
  for (int pos=0; pos<len; pos++) {
    QChar c = line[pos];
    if (bit=="") {
      bit = c;
      inword = c.isLetterOrNumber();
    } else if ((inword==c.isLetterOrNumber())
               || (inword && c==QChar('.')
                   && pos<len-1 && line[pos+1].isNumber())) {
        bit += c;
    } else {
      bits += bit;
      bit = c;
      inword = c.isLetterOrNumber();      
    }
  }
  if (bit != "")
    bits += bit;

  QRegularExpression presym("[*/+−=]");
  QRegularExpression postsym("[*/+−=_^]");
  QList<bool> mathcontext;
  for (int k=0; k<bits.size(); k++)  
    mathcontext << (bits[k].size()==1 && bits[k][0].isLetter()
                    && ((k>0 && bits[k-1].contains(presym))
                        || (k+1<bits.size() && bits[k+1].contains(postsym))));
  
  bool insup = false;
  bool insub = false;
  for (int k=0; k<bits.size(); k++) {
    QString bit = bits[k];
    //bit.replace("&", "&amp;"); // writeCharacters takes care of this
    //bit.replace("<", "&lt;");
    //bit.replace(">", "&gt;");
    if (mathcontext[k]) {
      sw.writeStartElement("tspan");
      sw.writeAttribute("style", "font-style: italic");
      sw.writeCharacters(bit);
      sw.writeEndElement();
      continue;
    }
    bit.replace(minus, "−");
    if (allnames.contains(bit)) {
      QString pfx = PartNumbering::prefix(bit);
      QString sfx = bit.mid(pfx.size());
      sw.writeStartElement("tspan");
      sw.writeAttribute("style", "font-style: italic");
      sw.writeCharacters(pfx);
      sw.writeEndElement();
      if (!sfx.isEmpty()) {
        sw.writeStartElement("tspan");
        sw.writeAttribute("style", QString("font-size: %1").arg(svgFontSize(true)));
        sw.writeAttribute("baseline-shift", SUBDISPLACE);
        sw.writeCharacters(sfx);
        sw.writeEndElement();
      }
    } else if ((bit.startsWith("V") || bit.startsWith("I"))
               && allnames.contains(bit.mid(1))) {
      QString wht = bit.left(1);
      QString name = bit.mid(1);
      QString pfx = PartNumbering::prefix(name);
      QString sfx = name.mid(pfx.size());
      sw.writeStartElement("tspan");
      sw.writeAttribute("style", "font-style: italic");
      sw.writeCharacters(wht);
      sw.writeEndElement();
      sw.writeStartElement("tspan");
      sw.writeAttribute("style", QString("font-size: %1").arg(svgFontSize(true)));
      sw.writeAttribute("baseline-shift", SUBDISPLACE);
      sw.writeStartElement("tspan");
      sw.writeAttribute("style", "font-style: italic");
      sw.writeCharacters(pfx);
      sw.writeEndElement();
      if (!sfx.isEmpty()) {
        sw.writeStartElement("tspan");
        sw.writeAttribute("style", QString("font-size: %1").arg(svgFontSize(true)*.7));
        sw.writeAttribute("baseline-shift", SUBSUBDISPLACE);
        sw.writeCharacters(sfx);
        sw.writeEndElement();
      }
      sw.writeEndElement();
 
    } else if (bit=="^") {
      if (insub) {
        sw.writeEndElement(); // close the tspan
        insub = false;
      }
      if (!insup) {
        sw.writeStartElement("tspan");
        sw.writeAttribute("style", QString("font-size: %1").arg(svgFontSize(true)));
        sw.writeAttribute("baseline-shift", SUPDISPLACE);
        insup = true;
      }
    } else if (bit=="_") {
      if (insup) {
        sw.writeEndElement(); // close the tspan
        insup = false;
      }
      if (!insub) {
        sw.writeStartElement("tspan");
        sw.writeAttribute("style", QString("font-size: %1").arg(svgFontSize(true)));
        sw.writeAttribute("baseline-shift", SUBDISPLACE);
        insub = true;
      }
    } else {
      sw.writeCharacters(bit);
      if (insup) {
        sw.writeEndElement();
        insup = false;
      }
      if (insub) {
        sw.writeEndElement();
        insub = false;
      }
    }
  }
  if (insup) {
    sw.writeEndElement();
    insup = false;
  }
  if (insub) {
    sw.writeEndElement();
    insub = false;
  }
}

void SvgExporterData::writeTextual(QXmlStreamWriter &sw,
				   Textual const &txt) {
  QPointF p = lib.upscale(txt.position) + svgFontDelta();
  float dy = 0;
  sw.writeStartElement("text");
  sw.writeAttribute("style", svgFontStyle(false, false, true));
  for (QString line: txt.text.split("\n")) {
    sw.writeStartElement("tspan");
    sw.writeAttribute("y", QString::number(p.y() + dy));
    sw.writeAttribute("x", QString::number(p.x()));
    writeTextualLine(sw, line);
    sw.writeEndElement();
    dy += 25;
  }
  sw.writeEndElement();
}

void SvgExporterData::writeConnection(QXmlStreamWriter &sw,
				      Connection const &con) {
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
  QRectF bbox = geom.visualBoundingRect();
  double s = lib.scale();
  double mrg = 1*s;
  bbox += QMarginsF(mrg, mrg, mrg, mrg);

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
  QFile file(fn);
  if (file.open(QFile::WriteOnly)) {
    QXmlStreamWriter sw(&file);
    sw.setAutoFormatting(false);
    //    sw.setAutoFormattingIndent(0);
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
    sw.writeAttribute("style", style);
    
    for (Element const &elt: d->circ.elements)
      d->writeElement(sw, elt);
    for (Connection const &con: d->circ.connections)
      d->writeConnection(sw, con);
    for (Textual const &txt: d->circ.textuals)
      d->writeTextual(sw, txt);

    
    sw.writeEndElement(); // g
    sw.writeEndElement(); // svg
    sw.writeEndDocument();
  } else {
    qDebug() << "Failed to export" << fn;
    return false;
  }
  return true;
}

