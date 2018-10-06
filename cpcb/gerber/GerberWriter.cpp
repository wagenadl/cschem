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
    layout(layout), dir(outputdir), collector(layout.board()), metric(false) {
  }
public:
  // Following return true if OK
  bool writeBoardOutline();
  bool writeThroughHoles();
  bool writeThroughHolesGerber();
  bool writeThroughHolesExcellon();
  bool writeCopper(Gerber::Layer);
  bool writePasteMask(Gerber::Layer);
  bool writeSolderMask(Gerber::Layer);
  bool writeSilk();
  static Layer mapLayer(Gerber::Layer l);
private:
  bool writeFilledPlanes(GerberFile &out, Gerber::Layer);
  bool writeTracksAndPads(GerberFile &out, Gerber::Layer);
  bool writeText(GerberFile &out, Gerber::Layer);
  bool writeTrackAndPadClearance(GerberFile &out, Gerber::Layer);
  bool writeTextClearance(GerberFile &out, Gerber::Layer);
  void collectNonCopperApertures(GerberFile &out, Gerber::Layer);
  void collectCopperApertures(GerberFile &out, Gerber::Layer);
  void collectCopperClearanceApertures(GerberFile &out, Gerber::Layer);
public:
  Layout layout;
  QDir dir;
  QString uuid;
  Collector collector;
  bool metric;
};

Layer GWData::mapLayer(Gerber::Layer l) {
  switch (l) {
  case Gerber::Layer::TopSilk: return Layer::Silk;
  case Gerber::Layer::TopCopper: return Layer::Top;
  case Gerber::Layer::TopPasteMask: return Layer::Top;
  case Gerber::Layer::TopSolderMask: return Layer::Top;
  case Gerber::Layer::BottomCopper: return Layer::Bottom;
  case Gerber::Layer::BottomPasteMask: return Layer::Bottom;
  case Gerber::Layer::BottomSolderMask: return Layer::Bottom;
  default: return Layer::Invalid;
  }
}


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
    return d->writeCopper(layer);
  case Gerber::Layer::BottomPasteMask: case Gerber::Layer::TopPasteMask:
    return d->writePasteMask(layer);
  case Gerber::Layer::BottomSolderMask: case Gerber::Layer::TopSolderMask:
    return d->writeSolderMask(layer);
  case Gerber::Layer::TopSilk:
    return d->writeSilk();
  default:
    qDebug() << "Unknown layer" << int(layer);
    return false;
  }
}

bool GWData::writeBoardOutline() {
  GerberFile out(dir, Gerber::Layer::BoardOutline, uuid);
  if (!out.isValid()) {
    qDebug() << "Could not create gerberfile for outline";
    return false;
  }
  out << "%TA.AperFunction,Profile*%\n";
  out << "%ADD10C,0.10000*%\n"; // create 0.1 mm thick line
  out << "G01*\n"; // linear
  out << "G75*\n"; // multisegment arcs
  out << "%LPD*%\n"; // positive
  out << "D10*\n"; // use aperture
  Board const &brd(layout.board());
  switch (brd.shape) {
  case Board::Shape::Rect:
    out << "X0Y0D02*\n"; // move to origin
    out << "X" << Gerber::coord(brd.width) << "D01*\n"; 
    out << "Y" << Gerber::coord(brd.height) << "D01*\n";
    out << "X0D01*\n";
    out << "Y0D01*\n"; // return to origin
    break;
  case Board::Shape::Round:
    if (brd.width==brd.height) {
      // simple circle
      out << "G01" << "X0Y" << Gerber::coord(brd.height/2) << "D02*\n";
      out << "G03" << "X0Y" << Gerber::coord(brd.height/2)
	  << "I" <<  Gerber::coord(brd.width/2)
	  << "J0" << "D01*\n";
    } else if (brd.width<brd.height) {
    } else {
    }
    break;
  }
  out << "M02*\n"; // terminate file
  return true;
}

bool GWData::writeThroughHoles() {
  return writeThroughHolesExcellon();
}

bool GWData::writeThroughHolesExcellon() {
  QFile f(dir.absoluteFilePath(dir.dirName()
			       + "-" + layerInfix(Gerber::Layer::ThroughHoles)
			       + ".TXT"));
  if (!f.open(QFile::WriteOnly)) {
    qDebug() << "Could not create file" << f.fileName();
    return false;
  }
  QTextStream out(&f);
  
  // Output header
  out << "; PTH drill file created by cpcb\n";
  out << "M48\n";
  if (metric)
    out << "METRIC,LZ\n";
  else
    out << "INCH,LZ\n";
  out << "FMAT,2\n";
  int toolidx = 0;
  QMap<Dim, int> toolindex;
  for (Dim d: collector.holes().keys()) {
    ++toolidx;
    toolindex[d] = toolidx;
    if (metric)
      out << QString("T%1C%2\n")
	.arg(toolidx, 2, 10, QChar('0'))
	.arg(d.toMM(), 0, 'f', 3);
    else
      out << QString("T%1C%2\n")
	.arg(toolidx, 2, 10, QChar('0'))
	.arg(d.toInch(), 0, 'f', 4);
  }
  out << "%\n";

  // Output body
  if (metric)
    out << "M71\n"; // explicit mm
  else
    out << "M72\n"; // explicit inch
  out << "G05\n";
  out << "G90\n";
  for (Dim d: collector.holes().keys()) {
    out << QString("T%1\n").arg(toolindex[d], 2, 10, QChar('0'));
    for (Hole const &h: collector.holes()[d]) {
      if (h.isSlot()) {
        Segment s(h.slotEnds());
        if (metric) 
          out << QString("X%1Y%2G85X%3Y%4\n")
            .arg(int(round(s.p1.x.toMM()*1000)), 6, 10, QChar('0'))
            .arg(int(round(s.p1.y.toMM()*1000)), 6, 10, QChar('0'))
            .arg(int(round(s.p2.x.toMM()*1000)), 6, 10, QChar('0'))
            .arg(int(round(s.p2.y.toMM()*1000)), 6, 10, QChar('0'));
        else
          out << QString("X%1Y%2G85X%3Y%4\n")
            .arg(int(round(s.p1.x.toMils()*10)), 6, 10, QChar('0'))
            .arg(int(round(s.p1.y.toMils()*10)), 6, 10, QChar('0'))
            .arg(int(round(s.p2.x.toMils()*10)), 6, 10, QChar('0'))
            .arg(int(round(s.p2.y.toMils()*10)), 6, 10, QChar('0'));
      } else {
        if (metric)
          out << QString("X%1Y%2\n")
            .arg(int(round(h.p.x.toMM()*1000)), 6, 10, QChar('0'))
            .arg(int(round(h.p.y.toMM()*1000)), 6, 10, QChar('0'));
        else 
          out << QString("X%1Y%2\n")
            .arg(int(round(h.p.x.toMils()*10)), 6, 10, QChar('0'))
            .arg(int(round(h.p.y.toMils()*10)), 6, 10, QChar('0'));
      }
    }
  }
  out << "M30\n";
  return true;
}

bool GWData::writeThroughHolesGerber() {
  GerberFile out(dir, Gerber::Layer::ThroughHoles, uuid);
  if (!out.isValid())
    return false;

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
    for (Hole const &h: collector.holes()[d]) {
      if (h.slotlength.isPositive())
        qDebug() << "Warning: Slotting not yet supported in Gerber drill files";
      out << Gerber::point(h.p) << "D03*\n";
    }
  }
  out << "M02*\n"; // terminate file
  return true;
}

bool GWData::writeSolderMask(Gerber::Layer layer) {
  GerberFile out(dir, layer, uuid);
  if (!out.isValid())
    return false;

  collectNonCopperApertures(out, layer);
  if (!writeTracksAndPads(out, layer))
    return false;

  out << "M02*\n";
  return true;
}

bool GWData::writeSilk() {
  GerberFile out(dir, Gerber::Layer::TopSilk, uuid);
  if (!out.isValid())
    return false;

  collectNonCopperApertures(out, Gerber::Layer::TopSilk);
  if (!writeTracksAndPads(out, Gerber::Layer::TopSilk))
    return false;
  if (!writeText(out, Gerber::Layer::TopSilk))
    return false;
  
  out << "M02*\n";
  return true;
}

bool GWData::writeCopper(Gerber::Layer layer) {
  GerberFile out(dir, layer, uuid);
  if (!out.isValid())
    return false;

  collectCopperClearanceApertures(out, layer);
  collectCopperApertures(out, layer);
  
  if (!writeFilledPlanes(out, layer))
    return false;
  if (!writeTrackAndPadClearance(out, layer))
    return false;
  if (!writeTextClearance(out, layer))
    return false;
  if (!writeTracksAndPads(out, layer))
    return false;
  if (!writeText(out, layer))
    return false;
  
  out << "M02*\n";
  return true;
}

bool GWData::writePasteMask(Gerber::Layer layer) {
  GerberFile out(dir, layer, uuid);
  if (!out.isValid())
    return false;

  collectCopperApertures(out, layer);
  if (!writeTracksAndPads(out, layer))
    return false;
  
  out << "M02*\n";
  return true;
}

bool GWData::writeFilledPlanes(GerberFile &out, Gerber::Layer layer) {

  for (Polyline const &pp: collector.filledPlanes(mapLayer(layer))) {
    if (pp.isEmpty())
      continue;
    out << "G36*\n"; // begin area
    out << Gerber::point(pp.last()) << "D02*\n";
    out << "G01*\n";
    for (Point const &p: pp) 
      out << Gerber::point(p) << "D01*\n";
    out << "G37*\n";
  }
  return true;
}

void GWData::collectNonCopperApertures(GerberFile &out, Gerber::Layer layer) {
  Gerber::Apertures
    &aps(out.newApertures(Gerber::Apertures::Func::Material));

  qDebug() << "cnca" << int(layer);
  for (Gerber::FontSpec spec: collector.texts(mapLayer(layer)).keys())
    qDebug() << " fs " << spec;
  for (Gerber::FontSpec spec: collector.texts(mapLayer(layer)).keys()) 
    out.ensureFont(spec);

  switch (layer) {
  case Gerber::Layer::TopSilk:
    for (Dim lw: collector.traces(mapLayer(layer)).keys())
      aps.ensure(Gerber::Circ(lw));
    for (Point p: collector.smdPads(mapLayer(layer)).keys())
      aps.ensure(Gerber::Rect(p.x, p.y));
    for (Dim lw: collector.arcs(mapLayer(layer)).keys())
      aps.ensure(Gerber::Circ(lw));
    break;
  case Gerber::Layer::TopSolderMask:
  case Gerber::Layer::BottomSolderMask:
    for (Dim lw: collector.traces(mapLayer(layer)).keys()) {
      Dim mrg = 2*layout.board().maskMargin(lw);
      aps.ensure(Gerber::Circ(lw + mrg));
    }
    for (Dim od: collector.roundHolePads(mapLayer(layer)).keys()) {
      Dim mrg = 2*layout.board().maskMargin(od);
      aps.ensure(Gerber::Circ(od + mrg));
    }
    for (Dim od: collector.squareHolePads(mapLayer(layer)).keys()) {
      Dim mrg = 2*layout.board().maskMargin(od);
      aps.ensure(Gerber::Rect(od + mrg, od + mrg));
    }
    for (Point p: collector.smdPads(mapLayer(layer)).keys()) {
      Dim mrg = 2*layout.board().maskMargin(p.x, p.y);
      aps.ensure(Gerber::Rect(p.x + mrg, p.y + mrg));
    }
    break;
  default:
    break;
  }
  out.writeApertures(aps);
}

void GWData::collectCopperClearanceApertures(GerberFile &out,
					     Gerber::Layer layer) {
  Layer l = mapLayer(layer);
  Gerber::Apertures
    &aps(out.newApertures(Gerber::Apertures::Func::AntiPad));
  for (Dim lw: collector.traces(l).keys())
    aps.ensure(Gerber::Circ(lw + 2*layout.board().traceClearance(lw)));
  for (Dim od: collector.roundHolePads(l).keys())
    aps.ensure(Gerber::Circ(od + 2*layout.board().padClearance(od, od)));
  for (Dim od: collector.squareHolePads(l).keys()) {
    Dim mrg(2*layout.board().padClearance(od, od));
    aps.ensure(Gerber::Rect(od + mrg, od + mrg));
  }
  for (Point p: collector.smdPads(l).keys()) {
    Dim mrg(2*layout.board().padClearance(p.x, p.y));
    aps.ensure(Gerber::Rect(p.x, p.y));
  }
  out.writeApertures(aps);
}

void GWData::collectCopperApertures(GerberFile &out, Gerber::Layer layer) {
  Layer l = mapLayer(layer);
  
  bool ispaste = layer==Gerber::Layer::TopPasteMask
    || layer==Gerber::Layer::BottomPasteMask;
  if (!ispaste) { // Apertures for traces
    Gerber::Apertures
      &aps(out.newApertures(Gerber::Apertures::Func::Conductor));
    for (Dim lw: collector.traces(l).keys())
      aps.ensure(Gerber::Circ(lw));
    for (Dim lw: collector.arcs(l).keys())
      aps.ensure(Gerber::Circ(lw));
    for (Dim od: collector.roundHolePads(l).keys())
      aps.ensure(Gerber::Rect(layout.board().fpConWidth(od, od)));
    for (Dim od: collector.squareHolePads(l).keys())
      aps.ensure(Gerber::Rect(layout.board().fpConWidth(od, od)));
    for (Point wh: collector.smdPads(l).keys())
      aps.ensure(Gerber::Rect(layout.board().fpConWidth(wh.x, wh.y)));
    out.writeApertures(aps);
  }

  if (!ispaste) { // Apertures for component pads
    Gerber::Apertures
      &aps(out.newApertures(Gerber::Apertures::Func::ComponentPad));
    for (Dim od: collector.roundHolePads(l).keys())
      aps.ensure(Gerber::Circ(od));
    for (Dim od: collector.squareHolePads(l).keys())
      aps.ensure(Gerber::Rect(od, od));
    out.writeApertures(aps);
  }
  
  { // Apertures for SMD pads
    Gerber::Apertures
      &aps(out.newApertures(Gerber::Apertures::Func::SMDPad));
    for (Point p: collector.smdPads(l).keys())
      aps.ensure(Gerber::Rect(p.x, p.y));
    out.writeApertures(aps);
  }

  qDebug() << "cca" << int(layer);
  for (Gerber::FontSpec spec: collector.texts(mapLayer(layer)).keys())
    qDebug() << " fs " << spec;
  for (Gerber::FontSpec spec: collector.texts(mapLayer(layer)).keys()) 
    out.ensureFont(spec);
  out.writeApertures(Gerber::Apertures::Func::Material);
}

bool GWData::writeTextClearance(GerberFile &out, Gerber::Layer layer) {
  auto const &txts = collector.texts(mapLayer(layer));
  SimpleFont const &sf(SimpleFont::instance());
  out << "G01*\n"; // linear
  out << "%LPC*%\n"; // negative
  for (Gerber::FontSpec spec: txts.keys()) {
    Dim mrg = layout.board().traceClearance(sf.scaleFactor(spec.fs)
                                            *sf.lineWidth());
    //    Gerber::Font const &font(out.font(spec));
    for (Text const &txt: txts[spec]) {
      Rect r(txt.boundingRect());
      r.grow(mrg);
      out << "G36*\n";
      out << Gerber::point(Point(r.left, r.top)) << "D02*\n";
      out << "G01*\n";
      out << "X" << Gerber::coord(r.right()) << "D01*\n";
      out << "Y" << Gerber::coord(r.bottom()) << "D01*\n";
      out << "X" << Gerber::coord(r.left) << "D01*\n";
      out << "Y" << Gerber::coord(r.top) << "D01*\n";
      out << "G37*\n";
    }
  }
  return true;
}

bool GWData::writeText(GerberFile &out, Gerber::Layer layer) {
  { // Output all text
    out << "G01*\n"; // linear
    out << "%LPD*%\n"; // positive
    qDebug() << "writetext" << int(layer);
    auto const &txts = collector.texts(mapLayer(layer));
    for (Gerber::FontSpec spec: txts.keys()) {
      qDebug() << " fs" << spec;
      Gerber::Font const &font(out.font(spec));
      for (Text const &txt: txts[spec]) {
        qDebug() << "txt " << txt;
        out << "G04 text " << txt.text << "*\n";
	font.writeText(out, txt);
      }
    }
  }
  return true;
}

static void writeRotatedRect(GerberFile &out,
                             Point const &p, Dim dx, Dim dy,
                             FreeRotation rota) {
  auto pt = [&](Point dp) { return Gerber::point(p + dp.rotated(rota)/2); };
  out << "G01*\n"; // linear
  out << "G36*\n"; // linear
  out << pt(Point(dx, dy)) << "D02*\n";
  out << pt(Point(-dx, dy)) << "D01*\n";
  out << pt(Point(-dx, -dy)) << "D01*\n";
  out << pt(Point(dx, -dy)) << "D01*\n";
  out << pt(Point(dx, dy)) << "D01*\n";
  out << "G37*\n";
}  

static void writeSlotSegment(GerberFile &out, Hole const &hole) {
  Segment s = hole.slotEnds();
  out << Gerber::point(s.p1) << "D02*\n";
  out << Gerber::point(s.p2) << "D01*\n";
}

bool GWData::writeTrackAndPadClearance(GerberFile &out, Gerber::Layer layer) {
  Layer l = mapLayer(layer);
  
  Gerber::Apertures const
    &aps(out.apertures(Gerber::Apertures::Func::AntiPad));
  out << "G01*\n"; // linear
  out << "%LPC*%\n"; // negative

  // Traces
  auto const &trcs(collector.traces(l));
  for (Dim lw: trcs.keys()) {
    Dim mrg = 2*layout.board().traceClearance(lw);
    out << aps.select(Gerber::Circ(lw + mrg));
    for (Trace const &trc: trcs[lw]) {
      out << Gerber::point(trc.p1) << "D02*\n";
      out << Gerber::point(trc.p2) << "D01*\n";
    }
  }

  // Through-hole component pads
  for (Dim od: collector.roundHolePads(l).keys()) {
    Dim mrg = 2*layout.board().padClearance(od, od);
    out << aps.select(Gerber::Circ(od + mrg));
    for (Hole const &hole: collector.roundHolePads(l)[od]) {
      if (!hole.noclear) {
        if (hole.isSlot()) 
          writeSlotSegment(out, hole);
        else 
          out << Gerber::point(hole.p) << "D03*\n";
      }
    }
  }
  for (Dim od: collector.squareHolePads(l).keys()) {
    Dim mrg = 2*layout.board().padClearance(od, od);
    out << aps.select(Gerber::Rect(od + mrg, od + mrg));
    for (Hole const &hole: collector.squareHolePads(l)[od]) {
      if (!hole.noclear) {
        if (hole.isSlot()) {
          if (hole.rota%90) {
            writeRotatedRect(out,
                             hole.p,
                             hole.slotlength + hole.od, hole.od,
                             hole.rota);
          } else {
            Segment s = hole.slotEnds();
            out << Gerber::point(s.p1) << "D02*\n";
            out << Gerber::point(s.p2) << "D01*\n";
          }
        } else {
          out << Gerber::point(hole.p) << "D03*\n";
        }
      }
    }
  }

  // SMD pads
  auto const &pads(collector.smdPads(l));
  for (Point p: pads.keys()) {
    Dim mrg = 2*layout.board().padClearance(p.x, p.y);
    out << aps.select(Gerber::Rect(p.x + mrg, p.y + mrg));
    for (Pad const &pad: pads[p]) {
      if (!pad.noclear) {
        if (pad.rota) 
          writeRotatedRect(out,
                           pad.p,
                           pad.width, pad.height,
                           pad.rota);
        else 
          out << Gerber::point(pad.p) << "D03*\n";
      }
    }
  }
  
  return true;
}

static void writeTraces(GerberFile &out, Gerber::Apertures const &aps,
                        QMap<Dim, QList<Trace>> const &trcs) {
  out << "G01*\n"; // linear
  out << "%LPD*%\n"; // positive
  for (Dim lw: trcs.keys()) {
    out << aps.select(Gerber::Circ(lw));
    for (Trace const &trc: trcs[lw]) {
      out << Gerber::point(trc.p1) << "D02*\n";
      out << Gerber::point(trc.p2) << "D01*\n";
    }
  }
}

static void writeArcs(GerberFile &out, Gerber::Apertures const &aps,
                        QMap<Dim, QList<Arc>> const &arcs) {
  constexpr double PI = 4*atan(1);
  out << "%LPD*%\n"; // positive
  out << "G75*\n"; // multiquadrant
  for (Dim lw: arcs.keys()) {
    out << aps.select(Gerber::Circ(lw));
    for (Arc const &arc: arcs[lw]) {
      double astart = PI/180*(arc.rota + 90);
      double aend = astart + PI/180*arc.angle;
      out << "G01" << Gerber::point(arc.center
                                    + Point(arc.radius*cos(astart),
                                            arc.radius*sin(astart)))
          << "D02*\n";
      out << "G03"
          << Gerber::point(arc.center
                           + Point(arc.radius*cos(aend),
                                   arc.radius*sin(aend)))
          << "I" << Gerber::coord(-arc.radius*cos(astart))
          << "J" << Gerber::coord(-arc.radius*sin(astart))
          << "D01*\n";
    }
  }
  out << "G01*\n";
}

class PointRota {
public:
  PointRota(Point const &p, FreeRotation const &r, Dim const &x=Dim()):
    p(p), r(r), x(x) {}
  Point p;
  FreeRotation r;
  Dim x;
};

static void writeFPCons(GerberFile &out, Gerber::Apertures const &trcaps,
                        Board const &brd, Dim w, Dim h,
                        QList<PointRota> const &lst) {
  Dim fpcw = brd.fpConWidth(w, h);
  Dim dxm = w/2 + brd.padClearance(w, h) - fpcw/2
    + Board::fpConOverlap();
  Dim dym = h/2 + brd.padClearance(w, h) - fpcw/2
    + Board::fpConOverlap();
  out << trcaps.select(Gerber::Rect(fpcw));
  for (auto const &pr: lst) {
    auto pt = [&](Dim dx, Dim dy) {
      return Gerber::point(pr.p + Point(dx, dy).rotated(pr.r));
    };
    out << pt(-dxm-pr.x/2, Dim()) << "D02*\n";
    out << pt(dxm+pr.x/2, Dim()) << "D01*\n";
    out << pt(Dim(), -dym) << "D02*\n";
    out << pt(Dim(), dym) << "D01*\n";
  }
}

static void writeFPCons(GerberFile &out, Gerber::Apertures const &trcaps,
                        Board const &brd, Dim w, Dim h,
                        QList<Pad> const &lst) {
  QList<PointRota> sel;
  for (Pad const &p: lst) 
    if (p.fpcon)
      sel << PointRota(p.p, p.rota);
  writeFPCons(out, trcaps, brd, w, h, sel);
}

static void writeFPCons(GerberFile &out, Gerber::Apertures const &trcaps,
                        Board const &brd, Dim w, Dim h,
                        QList<Hole> const &lst, Layer layer) {
  QList<PointRota> sel;
  for (Hole const &p: lst) 
    if (p.fpcon==layer)
      sel << PointRota(p.p, p.rota, p.slotlength);
  writeFPCons(out, trcaps, brd, w, h, sel);
}


static void writeComponentPads(GerberFile &out,
                               Board const &brd,
                               Collector const &col,
                               Layer l, bool sq,
                               bool mask) {
  Gerber::Apertures const
    &padaps(out.apertures(mask
                          ? Gerber::Apertures::Func::Material
                          : Gerber::Apertures::Func::ComponentPad));
  auto const &pads(sq ? col.squareHolePads(l) : col.roundHolePads(l));

  out << "G01*\n"; // linear
  out << "%LPD*%\n"; // positive
  for (Dim od: pads.keys()) {
    Dim d = od;
    if (mask)
      d += 2 * brd.maskMargin(od);
    if (sq)
      out << padaps.select(Gerber::Rect(d, d));
    else
      out << padaps.select(Gerber::Circ(d));
    for (Hole const &hole: pads[od]) {
      if (hole.isSlot()) {
        if (hole.rota && hole.square) {
          writeRotatedRect(out, hole.p,
                           hole.od + hole.slotlength, hole.od,
                           hole.rota);
        } else {
          writeSlotSegment(out, hole);
        }
      } else {
        out << Gerber::point(hole.p) << "D03*\n";
      }
    }
    if (!mask) {
      Gerber::Apertures const
        &trcaps(out.apertures(Gerber::Apertures::Func::Conductor));
      writeFPCons(out, trcaps, brd, od, od, pads[od], l);
    }
  }
}

static void writeSMDPads(GerberFile &out,
                         Board const &brd,
                         Collector const &col,
                         Layer l,
                         bool copper, bool mask) { // paste if neither
  Gerber::Apertures const
    &padaps(out.apertures(mask
                          ? Gerber::Apertures::Func::Material
                          : Gerber::Apertures::Func::SMDPad));
  auto const &pads(col.smdPads(l));
  out << "G01*\n"; // linear
  out << "%LPD*%\n"; // positive (even in mask layers!)
  for (Point p: pads.keys()) {
    Dim w(p.x);
    Dim h(p.y);
    Dim mrg = Dim();
    if (mask)
      mrg = brd.maskMargin(p.x, p.y);
    out << padaps.select(Gerber::Rect(w + 2*mrg, h + 2*mrg));
    for (Pad const &pad: pads[p]) {
      if (pad.rota) 
        writeRotatedRect(out, pad.p,
                         pad.width, pad.height, pad.rota);
      else 
        out << Gerber::point(pad.p) << "D03*\n";
    }
    if (copper) {
      Gerber::Apertures const
        &trcaps(out.apertures(Gerber::Apertures::Func::Conductor));
      writeFPCons(out, trcaps, brd, w, h, pads[p]);
    }
  }
}

bool GWData::writeTracksAndPads(GerberFile &out, Gerber::Layer layer) {
  Layer l = mapLayer(layer);
  bool iscopper = layer==Gerber::Layer::TopCopper
    || layer==Gerber::Layer::BottomCopper;
  bool ismask = layer==Gerber::Layer::TopSolderMask
    || layer==Gerber::Layer::BottomSolderMask;
  bool ispaste = layer==Gerber::Layer::TopPasteMask
    || layer==Gerber::Layer::BottomPasteMask;
  
  if (iscopper || layer==Gerber::Layer::TopSilk) {
    writeTraces(out,
                out.apertures(iscopper 
                              ? Gerber::Apertures::Func::Conductor
                              : Gerber::Apertures::Func::Material),
                collector.traces(l));
    writeArcs(out,
                out.apertures(iscopper 
                              ? Gerber::Apertures::Func::Conductor
                              : Gerber::Apertures::Func::Material),
                collector.arcs(l));
  }

  if (iscopper || ismask) {
    writeComponentPads(out, layout.board(), collector, l, false, ismask);
    writeComponentPads(out, layout.board(), collector, l, true, ismask);
  }

  if (iscopper || ismask || ispaste)
    writeSMDPads(out, layout.board(), collector, l, iscopper, ismask);

  return true;
}

bool GerberWriter::write(Layout const &layout, QString odir) {
  GerberWriter writer(layout, odir);
  if (!writer.prepareFolder())
    return false;
  return writer.writeAllLayers();
}

bool GerberWriter::writeAllLayers() {
  if (!writeLayer(Gerber::Layer::BoardOutline))
    return false;
  if (!writeLayer(Gerber::Layer::ThroughHoles))
    return false;
  if (!writeLayer(Gerber::Layer::BottomCopper))
    return false;
  if (!writeLayer(Gerber::Layer::BottomSolderMask))
    return false;
  if (!writeLayer(Gerber::Layer::BottomPasteMask))
    return false;
  if (!writeLayer(Gerber::Layer::TopCopper))
    return false;
  if (!writeLayer(Gerber::Layer::TopSolderMask))
    return false;
  if (!writeLayer(Gerber::Layer::TopPasteMask))
    return false;
  if (!writeLayer(Gerber::Layer::TopSilk))
    return false;

  return true;
}
