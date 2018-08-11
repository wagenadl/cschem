// GerberWriter.cpp

#include "GerberWriter.h"
#include "data/Board.h"
#include "GerberFile.h"
#include "Apertures.h"
#include "Collector.h"

#include <QUuid>
#include <QDir>

class GWData {
public:
  GWData(Layout const &layout, QString outputdir):
    layout(layout), dir(outputdir), collector(layout.board()) {
  }
public:
  // Following return true if OK
  bool writeBoardOutline();
  bool writeThroughHoles();
  bool writeCopper(Layer);
  bool writeSolderMask(Layer);
  bool writeSilk();
private:
  bool writeFilledPlanes(GerberFile &out, Layer);
  bool writeTracksAndPads(GerberFile &out, Layer, Gerber::Polarity);
  // negative polarity means to draw clearance, with appropriate widths
  bool writeText(GerberFile &out, Layer, Gerber::Polarity);
  void collectApertures(GerberFile &out, Layer, Gerber::Polarity);
public:
  Layout layout;
  QDir dir;
  QString uuid;
  Collector collector;
};

GerberWriter::GerberWriter(Layout const &layout, QString outputdir):
  d(new GWData(layout, outputdir)) {
  d->uuid = QUuid::createUuid().toString();
  d->uuid.replace("{", "");
  d->uuid.replace("}", "");
  d->collector.collect(layout.root());
}

GerberWriter::~GerberWriter() {
  delete d;
}

bool GerberWriter::prepareFolder() {
  // ensure the folder exists
  if (!d->dir.exists())
    QDir::root().mkpath(d->dir.absolutePath());
  if (!d->dir.exists()) {
    qDebug() << "Could not make dir";
    return false;
  }

  // remove all previous .gbr files in the folder
  QStringList filters; filters << "*.gbr";
  for (QString fn: d->dir.entryList(filters, QDir::Files)) {
    if (!d->dir.remove(fn)) {
      qDebug() << "Could not remove" << fn;
      return false;
    }
  }
  return true;
}

bool GerberWriter::writeLayer(Gerber::Layer layer) {
  switch (layer) {
  case Gerber::Layer::BoardOutline:
    return d->writeBoardOutline();
   case Gerber::Layer::ThroughHoles:
     return d->writeThroughHoles();
  case Gerber::Layer::BottomCopper: case Gerber::Layer::TopCopper:
    return d->writeCopper(layer==Gerber::Layer::BottomCopper
			  ? Layer::Bottom : Layer::Top);
  case Gerber::Layer::BottomSolderMask: case Gerber::Layer::TopSolderMask:
    return d->writeSolderMask(layer==Gerber::Layer::BottomSolderMask
			      ? Layer::Bottom : Layer::Top);
  case Gerber::Layer::TopSilk:
    return d->writeSilk();
  default:
    qDebug() << "Unknown layer" << int(layer);
    return false;
  }
}

bool GWData::writeBoardOutline() {
  GerberFile out(dir, Gerber::Layer::BoardOutline);
  if (!out.isValid()) {
    qDebug() << "Could not create gerberfile for outline";
    return false;
  }
  out << "%TF.FileFunction,Profile,NP*%\n";
  out.writeBoilerplate(uuid);
  out << "%TA.AperFunction,Profile*%\n";
  out << "%ADD10C,0.10000*%\n"; // not sure we really need this
  out << "G01*\n"; // linear
  out << "G75*\n"; // area
  out << "%LPD*%\n"; // positive
  out << "D10*\n"; // use apertures (but why?)
  out << "X0Y0D02*\n"; // move to origin
  out << "X" << Gerber::coord(layout.board().width) << "D01*\n"; 
  out << "Y" << Gerber::coord(layout.board().height) << "D01*\n";
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

  Gerber::Apertures &aps(out
			 .newApertures(Gerber::Apertures::Func::ComponentDrill));
  for (Dim d: collector.holes().keys())
    aps.ensure(Gerber::Circ(d));
  out << "%TA.DrillTolerance,0.10000,0.03000*%\n";
  out.writeApertures(aps);

  out << "G01*\n";
  out << "%LPD*%\n";

  for (Dim d: collector.holes().keys()) {
    out << aps.select(Gerber::Circ(d));
    for (Point p: collector.holes()[d]) 
      out << Gerber::point(p) << "D03*\n";
  }
  out << "M02*\n"; // terminate file
  return true;
}

bool GWData::writeSolderMask(Layer /*layer*/) {
  qDebug() << "writeSolderMask NYI";
  return true;
}

bool GWData::writeSilk() {
  GerberFile out(dir, Gerber::Layer::TopSilk);
  if (!out.isValid())
    return false;

  out << "%TF.FileFunction,Legend,Top*%\n";
  out.writeBoilerplate(uuid);

  collectApertures(out, Layer::Silk, Gerber::Polarity::Positive);
  if (!writeTracksAndPads(out, Layer::Silk, Gerber::Polarity::Positive))
    return false;
  if (!writeText(out, Layer::Silk, Gerber::Polarity::Positive))
    return false;
  
  return true;
}

bool GWData::writeCopper(Layer layer) {
  GerberFile out(dir,
		 layer==Layer::Top
		 ? Gerber::Layer::TopCopper
		 : Gerber::Layer::BottomCopper);
  if (!out.isValid())
    return false;
  
  out << "%TF.FileFunction,Copper,"
      << (layer==Layer::Top ? "L1,Top" : "L2,Bottom")
      << ",Mixed*%\n";
  out.writeBoilerplate(uuid);

  collectApertures(out, layer, Gerber::Polarity::Negative);
  collectApertures(out, layer, Gerber::Polarity::Positive);
  
  if (!writeFilledPlanes(out, layer))
    return false;

  if (!writeTracksAndPads(out, layer, Gerber::Polarity::Negative))
    return false;

  if (!writeTracksAndPads(out, layer, Gerber::Polarity::Positive))
    return false;

  if (!writeText(out, layer, Gerber::Polarity::Positive))
    return false;
  
  out << "M02*\n";
  return true;
}

bool GWData::writeFilledPlanes(GerberFile &/*out*/, Layer /*layer*/) {
  qDebug() << "GWData::writeFilledPlanes NYI";
  return true; // safe until filled planes are implemented elsewhere
}

void GWData::collectApertures(GerberFile &out, Layer layer,
			      Gerber::Polarity pol) {
  bool isneg = pol==Gerber::Polarity::Negative;
  if (isneg)
    return; // for now, since we have no filled planes yet

  { // Apertures and characters for fonts
    // This MUST be first, because GerberFile has started the NonConductor Aps.
    Gerber::Font &font(out.font());
    for (auto const &lst: collector.texts(layer)) 
      for (Text const &txt: lst) 
	font.ensure(txt.text);
    out.writeApertures(Gerber::Apertures::Func::NonConductor);
    font.writeFont(out);
  }
  
  { // Apertures for traces
    Gerber::Apertures &aps(out.newApertures(Gerber::Apertures::Func::Conductor));
    for (Dim lw: collector.traces(layer).keys())
      aps.ensure(Gerber::Circ(lw));
    out.writeApertures(aps);
  }

  if (layer==Layer::Top || layer==Layer::Bottom) {
    // Apertures for component pads
    Gerber::Apertures &aps(out.
			   newApertures(Gerber::Apertures::Func::ComponentPad));
    for (Dim od: collector.roundHolePads().keys())
      aps.ensure(Gerber::Circ(od));
    for (Dim od: collector.squareHolePads().keys())
      aps.ensure(Gerber::Rect(od, od));
    out.writeApertures(aps);
  }

  { // Apertures for SMD pads
    Gerber::Apertures &aps(out.
			   newApertures(Gerber::Apertures::Func::SMDPad));
    for (Point p: collector.smdPads(layer).keys())
      aps.ensure(Gerber::Rect(p.x, p.y));
    out.writeApertures(aps);
  }
}

bool GWData::writeText(GerberFile &out, Layer layer,
				Gerber::Polarity pol) {
  bool isneg = pol==Gerber::Polarity::Negative;
  if (isneg)
    return true; // for now, since we have no filled planes yet
  { // Output all text
    Gerber::Apertures const &aps(out
	 .apertures(Gerber::Apertures::Func::NonConductor));
    Gerber::Font const &font(out.font());
    out << "G01*\n"; // linear
    out << "%LPD*%\n"; // positive
    for (Dim fs: collector.texts(layer).keys())
      for (Text const &txt: collector.texts(layer)[fs])
	font.writeText(out, txt);
  }
  out << "%LMN*%\n";
  out << "%LS1.0*%\n";
  out << "%LR0*%\n";
  return true;
}

bool GWData::writeTracksAndPads(GerberFile &out, Layer layer,
				Gerber::Polarity pol) {
  bool isneg = pol==Gerber::Polarity::Negative;
  if (isneg)
    return true; // for now, since we have no filled planes yet

  
  { // Output all traces
    Gerber::Apertures const &aps(out
				 .apertures(Gerber::Apertures::Func::Conductor));
    out << "G01*\n"; // linear
    out << "%LPD*%\n"; // positive
    for (Dim lw: collector.traces(layer).keys()) {
      out << aps.select(Gerber::Circ(lw));
      for (Trace const &trc: collector.traces(layer)[lw]) {
	out << Gerber::point(trc.p1) << "D02*\n";
	out << Gerber::point(trc.p2) << "D01*\n";
      }
    }
  }

  { // Output all the component pads
    Gerber::Apertures const &aps(out
		 .apertures(Gerber::Apertures::Func::ComponentPad));
    out << "G01*\n"; // linear
    out << "%LPD*%\n"; // positive
    for (Dim od: collector.roundHolePads().keys()) {
      out << aps.select(Gerber::Circ(od));
      for (Point const &p: collector.roundHolePads()[od])
	out << Gerber::point(p) << "D03*\n";
    }
    for (Dim od: collector.squareHolePads().keys()) {
      out << aps.select(Gerber::Rect(od, od));
      for (Point const &p: collector.squareHolePads()[od])
	out << Gerber::point(p) << "D03*\n";
    }
  }

  { // Output all the SMD pads
    Gerber::Apertures const &aps(out
		 .apertures(Gerber::Apertures::Func::SMDPad));
    out << "G01*\n"; // linear
    out << "%LPD*%\n"; // positive
    for (Point p: collector.smdPads(layer).keys()) {
      out << aps.select(Gerber::Rect(p.x, p.y));
      for (Point const &p: collector.smdPads(layer)[p])
	out << Gerber::point(p) << "D03*\n";
    }
  }

  qDebug() << "GWData: Outputing arcs NYI";
  return true;
}

bool GerberWriter::write(Layout const &layout, QString odir) {
  GerberWriter writer(layout, odir);
  if (!writer.prepareFolder())
    return false;
  if (!writer.writeLayer(Gerber::Layer::BoardOutline))
    return false;
  if (!writer.writeLayer(Gerber::Layer::ThroughHoles))
    return false;
  if (!writer.writeLayer(Gerber::Layer::BottomCopper))
    return false;
  if (!writer.writeLayer(Gerber::Layer::BottomSolderMask))
    return false;
  if (!writer.writeLayer(Gerber::Layer::TopCopper))
    return false;
  if (!writer.writeLayer(Gerber::Layer::TopSolderMask))
    return false;
  if (!writer.writeLayer(Gerber::Layer::TopSilk))
    return false;

  return true;
}
