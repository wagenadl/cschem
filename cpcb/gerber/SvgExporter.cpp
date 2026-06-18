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
      writer.renderHole(obj.asHole());
      break;
    case Object::Type::NPHole:
      writer.renderNPHole(obj.asNPHole());
      break;
    case Object::Type::Pad:
      writer.renderPad(obj.asPad());
      break;
    case Object::Type::Arc:
      writer.renderArc(obj.asArc());
      break;
    case Object::Type::Trace:
      writer.renderTrace(obj.asTrace());
      break;
    case Object::Type::Plane:
      writer.renderPlane(obj.asPlane());
      break;
    default:
      // lots of other objects
      break;
    }
  }
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
