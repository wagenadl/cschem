// SvgExporter.cpp

#include "SvgExporter.h"
#include "SvgWriter.h"
#include "data/Layout.h"
#include "data/Object.h"
#include <QFile>
#include <QTextStream>
#include "data/Hole.h"
#include "data/Pad.h"
#include "data/Trace.h"

class SVGEHelper {
public:
  SVGEHelper(QFile *file, Board const &board):
    board(board),
    writer(file, board.width, board.height) {
  }
  void writeSvgOutline();
  void writeGroup(Group const &);
  void writePad(Pad const &);
  void writeHole(Hole const &);
  void writeTrace(Trace const &);
public:
  Board board;
  SvgWriter writer;
};
  
void SVGEHelper::writeSvgOutline() {
  writer.drawRect(Rect(Point(), Point(board.width, board.height)),
                  QColor(0,0,0));
}

void SVGEHelper::writeGroup(Group const &g) {
  for (int id: g.keys()) {
    Object const &obj(g.object(id));
    switch (obj.type()) {
    case Object::Type::Group:
      writeGroup(obj.asGroup());
      break;
    case Object::Type::Hole:
      writeHole(obj.asHole());
      break;
    case Object::Type::Pad:
      writePad(obj.asPad());
      break;
    case Object::Type::Trace:
      writeTrace(obj.asTrace());
      break;
    default:
      // lots of other objects
      break;
    }
  }
}

void SVGEHelper::writeTrace(Trace const &trace) {
  writer.drawTrace(trace.p1, trace.p2, trace.width, layerColor(trace.layer));
}
 
void SVGEHelper::writeHole(Hole const &hole) {
  // this needs to be improved
  writer.fillRing(hole.p, hole.id, hole.od, layerColor(Layer::Top));
}
 
void SVGEHelper::writePad(Pad const &pad) {
  writer << "<g transform=\"translate("
         << writer.coord(pad.p.x) << "," << writer.coord(pad.p.y) << "),"
         << "rotate(" << pad.rota.toString() << ")\">";
  Dim x0 = -pad.width/2;
  Dim y0 = -pad.height/2;
  Dim x1 = pad.width/2;
  Dim y1 = pad.height/2;
  writer.fillRect(Rect(Point(x0,y0), Point(x1,y1)), layerColor(pad.layer));
  writer << "</g>\n";
}

SvgExporter::SvgExporter() {
}

bool SvgExporter::write(Layout const &layout, QString filename) {
  QFile file(filename);
  if (!file.open(QFile::WriteOnly | QFile::Text))
    return false;
  
  SVGEHelper *d = new SVGEHelper(&file, layout.board());
  d->writeSvgOutline();
  d->writeGroup(layout.root());
  delete d;

  if (file.error() != QFile::NoError)
    return false;
  file.close();
  return true;
}
