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
#include "svg/PackageLibrary.h"

class GedaExporterData {
public:
  GedaExporterData(Schem const &schem):
    circ(schem.circuit()), lib(schem.library()),
    packaging(schem.packaging()),
    geom(circ, lib) {
  }
  void writeHeader(QTextStream &ts);
  void writeElement(QTextStream &ts, Element const &elt);
  void writeConnection(QTextStream &ts, Connection const &elt);
public:
  Circuit circ;
  SymbolLibrary lib;
  Packaging packaging;
  Geometry geom;
};

void GedaExporterData::writeHeader(QTextStream &ts) {
  ts << "v 20130925 2\n";
}

static QString gedapoint(QPoint p) {
  constexpr int OFFSET = 40000;
  return QString("%1 %2").arg(OFFSET+100*p.x()).arg(OFFSET-100*p.y());
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
  ts << QString("C %1 1 0 0 EMBEDDED-%2.sym\n")
    .arg(gedapoint(origin)).arg(elt.name);
  ts << "[\n";
  for (int k=0; k<pinnames.size(); k++) {
    ts << QString("P %1 %2 1 0 0\n")
      .arg(gedapoint(pinpos[k]))
      .arg(gedapoint((pinpos[k]+QPoint(1,0))));
    ts << "{\n";
    ts << QString("T %1 5 8 0 1 0 0 1\n")
      .arg(gedapoint(pinpos[k]+QPoint(1,-1)));
    ts << QString("pinseq=%1\n").arg(k+1); // this should be smarter
    ts << QString("T %1 5 8 0 1 0 0 1\n")
      .arg(gedapoint(pinpos[k]+QPoint(1,-2)));
    ts << QString("pinnumber=%1\n").arg(k+1); // this should be smarter
    ts << QString("T %1 5 8 0 1 0 0 1\n")
      .arg(gedapoint(pinpos[k]+QPoint(1,-3)));
    ts << QString("pinlabel=%1\n").arg(pinnames[k]); // this should be smarter
    ts << QString("T %1 5 8 0 1 0 0 1\n")
      .arg(gedapoint(pinpos[k]+QPoint(1,-4)));
    ts << QString("pintype=pas\n"); // this should be smarter
    ts << "}\n";
  }
  /*
  ts << QString("T %1 5 10 1 1 0 0 1\n")
    .arg(gedapoint(origin+QPoint(1,1)));
  ts << QString("refdes=%1\n").arg(elt.name);

  ts << QString("T %1 5 10 1 1 0 0 1\n")
    .arg(gedapoint(origin+QPoint(1,2)));
  ts << QString("device=%1\n").arg(elt.symbol());

  ts << QString("T %1 5 10 1 1 0 0 1\n")
    .arg(gedapoint(origin+QPoint(1,3)));
  ts << QString("pins=%1\n").arg(pinnames.size());

  ts << QString("T %1 5 10 1 1 0 0 1\n")
    .arg(gedapoint(origin+QPoint(1,4)));
  ts << QString("class=DISCRETE\n");
  */
  ts << "]\n";
  ts << "{\n";
  ts << QString("T %1 5 10 1 1 0 0 1\n")
    .arg(gedapoint(origin+QPoint(-1,1)));
  ts << QString("refdes=%1\n").arg(elt.name);

  if (!elt.value.isEmpty()) {
    ts << QString("T %1 5 10 1 1 0 0 1\n")
      .arg(gedapoint(origin+QPoint(-1,2)));
    ts << QString("value=%1\n").arg(elt.value);
  }
  ts << QString("T %1 5 10 1 1 0 0 1\n")
    .arg(gedapoint(origin+QPoint(-1,3)));
  QString pkgname = elt.info.package;
  QString pcb = packaging.packages.contains(pkgname)
    ? packaging.packages[pkgname].pcb : "???";
  ts << QString("footprint=%1\n").arg(pcb);

  ts << "}\n";
}

void GedaExporterData::writeConnection(QTextStream &ts,
				       Connection const &con) {
  QPolygon pp(geom.connectionPath(con));
  for (int n=0; n<pp.length()-1; n++)
    ts << QString("N %1 %2 4\n")
      .arg(gedapoint(pp[n])).arg(gedapoint(pp[n+1]));
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

