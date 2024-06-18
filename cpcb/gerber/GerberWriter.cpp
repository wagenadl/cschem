// GerberWriter.cpp

#include "GerberWriter.h"
#include "data/Board.h"
#include "GerberFile.h"
#include "Apertures.h"
#include "Collector.h"
#include "data/pi.h"

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
  bool writeNPHoles();
  bool writeCopper(Gerber::Layer);
  bool writePasteMask(Gerber::Layer);
  bool writeSolderMask(Gerber::Layer);
  bool writeSilk(Gerber::Layer);
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
  case Gerber::Layer::BottomSilk: return Layer::BSilk;
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
  case Gerber::Layer::NonplatedHoles:
    return d->writeNPHoles();
  case Gerber::Layer::BottomCopper: case Gerber::Layer::TopCopper:
    return d->writeCopper(layer);
  case Gerber::Layer::BottomPasteMask: case Gerber::Layer::TopPasteMask:
    return d->writePasteMask(layer);
  case Gerber::Layer::BottomSolderMask: case Gerber::Layer::TopSolderMask:
    return d->writeSolderMask(layer);
  case Gerber::Layer::TopSilk:
    return d->writeSilk(layer);
  case Gerber::Layer::BottomSilk:
    return d->writeSilk(layer);
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
  //  out << "%TA.AperFunction,Profile*%\n";
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
      // simple circle: use left edge as start/end
      out << "G01"
          << "X0Y" << Gerber::coord(brd.height/2) << "D02*\n";
      out << "G03"
          << "X0Y" << Gerber::coord(brd.height/2)
	  << "I" <<  Gerber::coord(brd.width/2)
	  << "J0" << "D01*\n";
    } else if (brd.width<brd.height) {
      // vertically oriented obrect. start at top of left edge
      out << "G01"
          << "X0Y" << Gerber::coord(brd.width/2) << "D02*\n";
      out << "G03"
          << "X" << Gerber::coord(brd.width)
          << "Y" << Gerber::coord(brd.width/2)
	  << "I" << Gerber::coord(brd.width/2) << "J0"
          << "D01*\n"; // draw top half circle
      out << "G01" 
          << "X" << Gerber::coord(brd.width)
          << "Y" << Gerber::coord(brd.height - brd.width/2)
          << "D01*\n"; // draw right edge
      out << "G03"
          << "X0"
          << "Y" << Gerber::coord(brd.height - brd.width/2)
	  << "I" << Gerber::coord(-brd.width/2) << "J0"
          << "D01*\n"; // draw bottom half circle
      out << "G01" 
          << "X0" << "Y" << Gerber::coord(brd.width/2)
          << "D01*\n"; // draw left edge
    } else {
      // horizontally oriented obrect. start at left of bottom edge
      out << "G01"
          << "X" << Gerber::coord(brd.height/2) << "Y0" << "D02*\n";
      out << "G02"
          << "X" << Gerber::coord(brd.height/2)
          << "Y" << Gerber::coord(brd.height)
	  << "I0" << "J" << Gerber::coord(brd.height/2)
          << "D01*\n"; // draw left half circle
      out << "G01" 
          << "X" << Gerber::coord(brd.width - brd.height/2)
          << "Y" << Gerber::coord(brd.height)
          << "D01*\n"; // draw top edge
      out << "G02"
          << "X" << Gerber::coord(brd.width - brd.height/2)
          << "Y0"
	  << "I0" << "J" << Gerber::coord(-brd.height/2)
          << "D01*\n"; // draw right half circle
      out << "G01" 
          << "X" << Gerber::coord(brd.height/2) << "Y0"
          << "D01*\n"; // draw bottom edge
    }
    break;
  }
  out << "M02*\n"; // terminate file
  return true;
}

static QString excellonMetric(Point p) {
  return QString("X%1Y%2")
    .arg(int(round(p.x.toMM()*1000)), 6, 10, QChar('0'))
    .arg(int(round(p.y.toMM()*1000)), 6, 10, QChar('0'));
}

static QString excellonInch(Point p) {
  return QString("X%1Y%2")
    .arg(int(round(p.x.toMils()*10)), 6, 10, QChar('0'))
    .arg(int(round(p.y.toMils()*10)), 6, 10, QChar('0'));
}

bool GWData::writeThroughHoles() {
  QFile f(dir.absoluteFilePath(dir.dirName()
       + "-" + layerInfix(Gerber::Layer::ThroughHoles)
       + "." + layerSuffix(Gerber::Layer::ThroughHoles)));
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
          out << excellonMetric(s.p1) << "G85" << excellonMetric(s.p2) << "\n";
        else
          out << excellonInch(s.p1) << "G85" << excellonInch(s.p2) << "\n";
      } else {
        if (metric)
          out << excellonMetric(h.p) << "\n";
        else 
          out << excellonInch(h.p) << "\n";
      }
    }
  }
  out << "M30\n";
  return true;
}

bool GWData::writeNPHoles() {
  QFile f(dir.absoluteFilePath(dir.dirName()
       + "-" + layerInfix(Gerber::Layer::NonplatedHoles)
       + "." + layerSuffix(Gerber::Layer::NonplatedHoles)));
  if (!f.open(QFile::WriteOnly)) {
    qDebug() << "Could not create file" << f.fileName();
    return false;
  }
  QTextStream out(&f);
  
  // Output header
  out << "; NPTH drill file created by cpcb\n";
  out << "M48\n";
  if (metric)
    out << "METRIC,LZ\n";
  else
    out << "INCH,LZ\n";
  out << "FMAT,2\n";
  int toolidx = 0;
  QMap<Dim, int> toolindex;
  for (Dim d: collector.npHoles().keys()) {
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
  for (Dim d: collector.npHoles().keys()) {
    out << QString("T%1\n").arg(toolindex[d], 2, 10, QChar('0'));
    for (NPHole const &h: collector.npHoles()[d]) {
      if (h.isSlot()) {
        Segment s(h.slotEnds());
        if (metric) 
          out << excellonMetric(s.p1) << "G85" << excellonMetric(s.p2) << "\n";
        else
          out << excellonInch(s.p1) << "G85" << excellonInch(s.p2) << "\n";
      } else {
        if (metric)
          out << excellonMetric(h.p) << "\n";
        else 
          out << excellonInch(h.p) << "\n";
      }
    }
  }
  out << "M30\n";
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

bool GWData::writeSilk(Gerber::Layer layer) {
  GerberFile out(dir, layer, uuid);
  if (!out.isValid())
    return false;

  collectNonCopperApertures(out, layer);
  if (!writeTracksAndPads(out, layer))
    return false;
  if (!writeText(out, layer))
    return false;
  
  out << "M02*\n";
  return true;
}

bool GWData::writeCopper(Gerber::Layer layer) {
  GerberFile out(dir, layer, uuid);
  if (!out.isValid())
    return false;

  bool hasFilledPlanes = !collector.filledPlanes(mapLayer(layer)).isEmpty();

  if (hasFilledPlanes)
    collectCopperClearanceApertures(out, layer);
  collectCopperApertures(out, layer);

  if (hasFilledPlanes) {
    if (!writeFilledPlanes(out, layer))
      return false;
    if (!writeTrackAndPadClearance(out, layer))
      return false;
    if (!writeTextClearance(out, layer))
      return false;
  }
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

  //  qDebug() << "cnca" << int(layer);
  //for (Gerber::FontSpec spec: collector.texts(mapLayer(layer)).keys())
  //  qDebug() << " fs " << spec;
  for (Gerber::FontSpec spec: collector.texts(mapLayer(layer)).keys()) 
    out.ensureFont(spec);

  switch (layer) {
  case Gerber::Layer::TopSilk:
  case Gerber::Layer::BottomSilk:
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
    aps.ensure(Gerber::Rect(p.x + mrg, p.y + mrg));
  }
  out.writeApertures(aps);
}

void GWData::collectCopperApertures(GerberFile &out, Gerber::Layer layer) {
  Layer l = mapLayer(layer);
  
  bool ispaste = layer==Gerber::Layer::TopPasteMask
    || layer==Gerber::Layer::BottomPasteMask;
  /*
  bool ismask = layer==Gerber::Layer::TopSolderMask
    || layer==Gerber::Layer::BottomSolderMask;
  */
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

  if (!ispaste) { // Apertures for component pads (hole pads)
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

  //  qDebug() << "cca" << int(layer);
  // for (Gerber::FontSpec spec: collector.texts(mapLayer(layer)).keys())
  //   qDebug() << " fs " << spec;
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
    // qDebug() << "writetext" << int(layer);
    auto const &txts = collector.texts(mapLayer(layer));
    for (Gerber::FontSpec spec: txts.keys()) {
      // qDebug() << " fs" << spec;
      Gerber::Font const &font(out.font(spec));
      for (Text const &txt: txts[spec]) {
        // qDebug() << "txt " << txt;
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
  auto pt = [&](Point dp) {
    return Gerber::point(p + dp.rotatedFreely(rota)/2);
  };
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

static void writeTraceClearance(GerberFile &out, Board const &board,
                                QMap<Dim, QList<Trace>> const &trcs,
                                Gerber::Apertures const &aps) {
  for (Dim lw: trcs.keys()) {
    Dim mrg = 2*board.traceClearance(lw);
    out << aps.select(Gerber::Circ(lw + mrg));
    for (Trace const &trc: trcs[lw]) {
      if (trc.noclear)
        continue;
      out << Gerber::point(trc.p1) << "D02*\n";
      out << Gerber::point(trc.p2) << "D01*\n";
    }
  }
}

static void writeRoundHoleClearance(GerberFile &out, Board const &board,
                                    QMap<Dim, QList<Hole>> const &holes,
                                    Gerber::Apertures const &aps) {
  for (Dim od: holes.keys()) {
    Dim mrg = 2*board.padClearance(od, od);
    out << aps.select(Gerber::Circ(od + mrg));
    for (Hole const &hole: holes[od]) {
      if (hole.noclear)
        continue;
      if (hole.isSlot()) 
        writeSlotSegment(out, hole);
      else 
        out << Gerber::point(hole.p) << "D03*\n";
    }
  }
}

static void writeSqHoleClearance(GerberFile &out, Board const &board,
                                 QMap<Dim, QList<Hole>> const &holes,
                                 Gerber::Apertures const &aps) {
  for (Dim od: holes.keys()) {
    Dim mrg = 2*board.padClearance(od, od);
    out << aps.select(Gerber::Rect(od + mrg, od + mrg));
    for (Hole const &hole: holes[od]) {
      if (hole.noclear)
        continue;
      if (hole.rota%90) {
        writeRotatedRect(out,
                         hole.p,
                         hole.slotlength + hole.od + mrg, hole.od + mrg,
                         hole.rota);
      } else if (hole.isSlot()) {
        Segment s = hole.slotEnds();
        out << Gerber::point(s.p1) << "D02*\n";
        out << Gerber::point(s.p2) << "D01*\n";
      } else {
        out << Gerber::point(hole.p) << "D03*\n";
      }
    }
  }
}

static void writeSMDClearance(GerberFile &out, Board const &board,
                              QMap<Point, QList<Pad>> const &pads,
                              Gerber::Apertures const &aps) {
  for (Point p: pads.keys()) {
    Dim mrg = 2*board.padClearance(p.x, p.y);
    out << aps.select(Gerber::Rect(p.x + mrg, p.y + mrg));
    for (Pad const &pad: pads[p]) {
      if (pad.noclear)
        continue;
      if (pad.rota) 
        writeRotatedRect(out,
                         pad.p,
                         pad.width + mrg, pad.height + mrg,
                         pad.rota);
      else 
        out << Gerber::point(pad.p) << "D03*\n";
    }
  }
}

bool GWData::writeTrackAndPadClearance(GerberFile &out, Gerber::Layer layer) {
  Layer l = mapLayer(layer);
  
  Gerber::Apertures const
    &aps(out.apertures(Gerber::Apertures::Func::AntiPad));
  out << "G01*\n"; // linear
  out << "%LPC*%\n"; // negative

  writeTraceClearance(out, layout.board(), collector.traces(l), aps);
  writeRoundHoleClearance(out, layout.board(), collector.roundHolePads(l), aps);
  writeSqHoleClearance(out, layout.board(), collector.squareHolePads(l), aps);
  writeSMDClearance(out, layout.board(), collector.smdPads(l), aps);
  
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
                        Board const &brd, Point wh,
                        QList<PointRota> const &lst) {
  Dim w = wh.x;
  Dim h = wh.y;  
  Dim fpcw = brd.fpConWidth(w, h);
  Dim dxm = w/2 + brd.padClearance(w, h) - fpcw/2
    + Board::fpConOverlap();
  Dim dym = h/2 + brd.padClearance(w, h) - fpcw/2
    + Board::fpConOverlap();
  out << trcaps.select(Gerber::Rect(fpcw));
  for (PointRota const &pr: lst) {
    auto pt = [&](Dim dx, Dim dy) {
      return Gerber::point(pr.p + Point(dx, dy).rotatedFreely(pr.r));
    };
    out << pt(-dxm-pr.x/2, Dim()) << "D02*\n";
    out << pt(dxm+pr.x/2, Dim()) << "D01*\n";
    out << pt(Dim(), -dym) << "D02*\n";
    out << pt(Dim(), dym) << "D01*\n";
  }
}

static void writeFPCons(GerberFile &out, Gerber::Apertures const &trcaps,
                        Board const &brd, Point wh,
                        QList<Pad> const &lst) {
  QList<PointRota> sel;
  for (Pad const &p: lst) 
    if (p.fpcon && !p.noclear)
      sel << PointRota(p.p, p.rota);
  writeFPCons(out, trcaps, brd, wh, sel);
}

static void writeFPCons(GerberFile &out, Gerber::Apertures const &trcaps,
                        Board const &brd, Point wh,
                        QList<Hole> const &lst, Layer layer) {
  QList<PointRota> sel;
  for (Hole const &p: lst) 
    if (p.fpcon==layer && !p.noclear)
      sel << PointRota(p.p, p.rota, p.slotlength);
  writeFPCons(out, trcaps, brd, wh, sel);
}

static void writePlainComponentPads(GerberFile &out,
                                    Gerber::Apertures const &padaps,
                                    Board const &brd,
                                    Dim od, QList<Hole> const &lst,
                                    Layer l, bool sq,
                                    bool mask) {
  if (mask)
    od += 2 * brd.maskMargin(od);
  if (sq)
    out << padaps.select(Gerber::Rect(od));
  else
    out << padaps.select(Gerber::Circ(od));
  
  for (Hole const &hole: lst) {
    if (mask && hole.via)
      continue;
    if (hole.rota && hole.square) {
      writeRotatedRect(out, hole.p,
                       od + hole.slotlength, od,
                       hole.rota);
    } else if (hole.isSlot()) {
      writeSlotSegment(out, hole);
    } else {
      out << Gerber::point(hole.p) << "D03*\n";
    }
  }
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
  QMap<Dim, QList<Hole>> const &pads(sq ? col.squareHolePads(l)
                                       : col.roundHolePads(l));
  
  out << "G01*\n"; // linear
  out << "%LPD*%\n"; // positive
  for (Dim od: pads.keys()) {
    writePlainComponentPads(out, padaps, brd, od, pads[od], l, sq, mask);
    if (!mask) {
      writeFPCons(out, out.apertures(Gerber::Apertures::Func::Conductor), brd,
                  Point(od, od), pads[od], l);
    }
  }
}

static void writePlainSMDPads(GerberFile &out,
                              Gerber::Apertures const &padaps,
                              Board const &brd,
                              Point wh, QList<Pad> const &pads,
                              bool copper, bool mask) { // paste if neither
  Dim w(wh.x);
  Dim h(wh.y);
  Dim mrg = Dim();
  if (mask)
    mrg = brd.maskMargin(w, h);
  Dim w1(w + 2*mrg);
  Dim h1(h + 2*mrg);

  out << padaps.select(Gerber::Rect(w1, h1));
  for (Pad const &pad: pads) {
    if (pad.rota) 
      writeRotatedRect(out, pad.p, w1, h1, pad.rota);
    else 
      out << Gerber::point(pad.p) << "D03*\n";
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
  QMap<Point, QList<Pad>> const &pads(col.smdPads(l));
  out << "G01*\n"; // linear
  out << "%LPD*%\n"; // positive (even in mask layers!)

  for (Point wh: pads.keys()) {
    writePlainSMDPads(out, padaps, brd, wh, pads[wh], copper, mask);
    if (copper) 
      writeFPCons(out, out.apertures(Gerber::Apertures::Func::Conductor), brd,
                  wh, pads[wh]);
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
  bool issilk = layer==Gerber::Layer::TopSilk
    || layer==Gerber::Layer::BottomSilk;
  
  if (iscopper || issilk) {
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
  qDebug() << "gerberwriter" << odir;
  if (!writer.prepareFolder())
    return false;
  qDebug() << "gerberwriter: folder prepd";
  bool res = writer.writeAllLayers();
  qDebug() << "gerberwriter: layers written" << res;
  return res;
}

bool GerberWriter::writeAllLayers() {
  if (!writeLayer(Gerber::Layer::BoardOutline))
    return false;
  if (!writeLayer(Gerber::Layer::ThroughHoles))
    return false;
  if (!writeLayer(Gerber::Layer::NonplatedHoles))
    return false;
  if (!writeLayer(Gerber::Layer::BottomCopper))
    return false;
  if (!writeLayer(Gerber::Layer::BottomSolderMask))
    return false;
  if (!writeLayer(Gerber::Layer::BottomPasteMask))
    return false;
  if (!writeLayer(Gerber::Layer::BottomSilk))
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
