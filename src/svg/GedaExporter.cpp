// GedaExporter.cpp

#include "GedaExporter.h"
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

constexpr int OFFSET = 400;

class GedaExporterData {
public:
  GedaExporterData(Schem const &schem):
    circ(schem.circuit()), lib(schem.library()),
    geom(circ, lib) {
  }
  void writeHeader(QTextStream &ts);
  void writeElement(QTextStream &ts, Element const &elt);
  void writeConnection(QTextStream &ts, Connection const &elt);
public:
  Circuit circ;
  SymbolLibrary lib;
  Geometry geom;
};

void GedaExporterData::writeHeader(QTextStream &ts) {
  ts << "v 20130925 2\n";
}

void GedaExporterData::writeElement(QTextStream &ts, Element const &elt) {
  QString sym = elt.symbol();
  if (sym.startsWith("port:"))
    return;
  if (sym.startsWith("junction:"))
    return;

  // we need to be smart about containers and contained objects...
      
  Symbol const &symbol = lib.symbol(sym);
  QStringList pinnames = symbol.pinNames();
  QPolygon pinpos;
  for (QString pn: pinnames)
    pinpos << geom.pinPosition(elt, pn);
  QRect bb = pinpos.boundingRect();
  QPoint origin = bb.topLeft();
  pinpos.translate(-origin);
  bb.translate(OFFSET,OFFSET);
  origin += QPoint(OFFSET,OFFSET);
  pinpos.translate(OFFSET,OFFSET);
  ts << QString("C %1 %2 1 0 0 EMBEDDED-%3.sym\n")
    .arg(origin.x()*100).arg(origin.y()*100)
    .arg(elt.name);
  ts << "[\n";
  for (int k=0; k<pinnames.size(); k++) {
    ts << QString("P %1 %2 %3 %4 1 0 0\n")
      .arg(pinpos[k].x()*100).arg(pinpos[k].y()*100)
      .arg(pinpos[k].x()*100).arg(pinpos[k].y()*100);
    ts << "{\n";
    ts << QString("T %1 %2 5 8 0 1 0 0 1\n")
      .arg(pinpos[k].x()*100+25).arg(pinpos[k].y()*100+25);
    ts << QString("pinseq=%1\n").arg(k+1); // this should be smarter
    ts << QString("T %1 %2 5 8 0 1 0 0 1\n")
      .arg(pinpos[k].x()*100+25).arg(pinpos[k].y()*100+25);
    ts << QString("pinnumber=%1\n").arg(k+1); // this should be smarter
    ts << QString("T %1 %2 5 8 0 1 0 0 1\n")
      .arg(pinpos[k].x()*100+25).arg(pinpos[k].y()*100+25);
    ts << QString("pinlabel=%1\n").arg(pinnames[k]); // this should be smarter
    ts << QString("T %1 %2 5 8 0 1 0 0 1\n")
      .arg(pinpos[k].x()*100+25).arg(pinpos[k].y()*100+25);
    ts << QString("pintype=pas\n"); // this should be smarter
    ts << "}\n";
  }
  ts << QString("T %1 %2 5 10 1 1 0 0 1\n")
    .arg(origin.x()*100-25).arg(origin.y()*100-5);
  ts << QString("device=DEVICE\n"); // this likely needs to be smarter
  ts << QString("T %1 %2 5 10 1 1 0 0 1\n")
    .arg(origin.x()*100-25).arg(origin.y()*100-25);
  ts << QString("refdes=%1\n").arg(elt.name);
  ts << QString("T %1 %2 5 10 1 1 0 0 1\n")
    .arg(origin.x()*100-25).arg(origin.y()*100-50);
  ts << QString("value=%1\n").arg(elt.value);
  ts << QString("T %1 %2 5 10 1 1 0 0 1\n")
    .arg(origin.x()*100-25).arg(origin.y()*100-75);
  ts << QString("footprint=ACY300\n"); // this obviously needs to be smarter
  ts << "]\n";
}

void GedaExporterData::writeConnection(QTextStream &ts,
				       Connection const &con) {
  QPolygon pp(geom.connectionPath(con));
  pp.translate(OFFSET, OFFSET);
  for (int n=0; n<pp.length()-1; n++)
    ts << QString("N %1 %2 %3 %4 4\n")
      .arg(pp[n].x()*100).arg(pp[n].y()*100)
      .arg(pp[n+1].x()*100).arg(pp[n+1].y()*100);
}

GedaExporter::GedaExporter(Schem const &schem):
  d(new GedaExporterData(schem)) {
}

GedaExporter::~GedaExporter() {
  delete d;
}

bool GedaExporter::exportGeda(QString const &fn) {
  qDebug() << "symbol names" << d->lib.symbolNames();
  QFile file(fn);
  if (file.open(QFile::WriteOnly)) {
    QTextStream ts(&file);
    d->writeHeader(ts);
    
    for (Element const &elt: d->circ.elements)
      d->writeElement(ts, elt);

    for (Connection const &con: d->circ.connections)
      d->writeConnection(ts, con);
  } else {
    qDebug() << "Failed to export" << fn;
    return false;
  }
  return true;
}

