// GerberWriter.cpp

#include "GerberWriter.h"
#include "data/Board.h"

#include <QUuid>

class GWData {
public:
  GWData(Layout const &layout, QString outputdir):
    layout(layout), dir(outputdir) {
  }
public:
  bool writeBoardOutline();
  bool writeThroughHoles();
  bool writeCopper();
  bool writeMask();
  bool writeSilk();
public:
  Layout layout;
  QDir dir;
  QString uuid;
  Collector collector;
};

GerberWriter::GerberWriter(Layout const &layout, QString outputdir):
  d(new GWData(layout, outputdir)) {
  d->uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
  d->collector.collect(layout.root());
}

GerberWriter::~GerberWriter() {
  delete d;
}

bool GerberWriter::prepareFolder() {
  // ensure the folder exists
  if (!d->dir.exists())
    QDir::root().mkpath(d->dir.absolutePath());
  if (!d->dir.exists())
    return false;

  // remove all previous .gbr files in the folder
  QStringList filters; filters << "*.gbr";
  for (QString fn: d->dir.entryList(filters, QDir::Files))
    if (!d->dir.remove(fn))
      return false;

  return true;
}

bool GerberWriter::writeLayer(Gerber::Layer layer) {
  switch (layer) {
  case Gerber::Layer::BoardOutline:
    return d->writeBoardOutline;
   case Gerber::Layer::ThroughHoles:
     return d->writeThroughHoles;
  case Gerber::Layer::BottomCopper: case Gerber::Layer::TopCopper:
    return d->writeCopper(layer==Gerber::Layer::BottomCopper
			  ? Layer::Bottom : Layer::Top);
  case Gerber::Layer::BottomCopper: case Gerber::Layer::TopCopper:
    return d->writeMask(layer==Gerber::Layer::BottomCopper
			? Layer::Bottom : Layer::Top);
  case Gerber::Layer::TopSilk:
    return d->writeSilk();
  default:
    return false;
  }
}

bool GWData::writeBoardOutline() {
  GerberFile out(dir, Gerber::Layer::BoardOutline);
  if (!out.isValid())
    return false;
  out << "%TF.FileFunction,Profile,NP*%\n";
  out.writeBoilerplate(uuid);
  out << "%TA.AperFunction,Profile*%\n";
  out << "%ADD10C,0.10000*%\n"; // not sure we really need this
  out << "G01*\n"; // linear
  out << "G75*\n"; // area
  out << "%LPD*%\n"; // positive
  out << "D10*\n"; // use aperture (but why?)
  out << "X0Y0D02*\n"; // move to origin
  out << "X" << coord(layout.board().width()) << "D01*\n"; 
  out << "Y" << coord(layout.board().height()) << "D01*\n";
  out << "X0D01*\n";
  out << "Y0D01*\n"; // return to origin
  out << "M02*\n"; // terminate file
  return true;
}

bool GWData::writeThroughHoles() {
  GerberFile out(dir, Gerber::Layer::ThroughHoles);
  if (!out.isValid())
    return false;
  out << "%TF.FileFunction,Plated,1,2,PTH,Drill*%\n";  
  out.writeBoilerplate(uuid);

  Aperture aps;
  for (Dim d: collector.holes().keys())
    aps.ensure(Circ(d));

  out << "%TA.DrillTolerance,0.10000,0.03000*%\n";
  out << "%TA.AperFunction,ComponentDrill*%\n";
  aps.write(out);

  out << "G01*\n";
  out << "%LPD*%\n";

  for (Dim d: collector.holes().keys()) {
    out << aps.select(Circ(d));
    for (Point p: collector.holes()[diam]) 
      out << "X" << coord(p.x()) << "Y" << coord(p.y()) << "D03*\n";
  }
  out << "M02*\n"; // terminate file
  return true;
}

bool GWData::writeCopper(Layer layer) {
  GerberFile out(dir, layer==Layer::Top ? Gerber::Layer::TopCopper
		 : Gerber::Layer::BottomCopper);
  if (!out.isValid())
    return false;
  out << "%TF.FileFunction,Copper,"
      << (layer==Layer::Top ? "L1,Top" : "L2,Bottom")
      << ",Mixed*%\n";
  out.writeBoilerplate(uuid);

  out << "M02*\n";
  return false; // not yet fully implemented...
}
