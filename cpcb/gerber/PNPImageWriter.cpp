// PNPImageWriter.cpp

#include "PNPImageWriter.h"
#include "SvgWriter.h"
#include "data/Layout.h"
#include "data/Object.h"
#include <QFile>
#include <QTextStream>
#include <cmath>
#include "data/PickNPlace.h"

class PIWHelper {
public:
  PIWHelper(QFile *file, Board const &board):
    board(board),
    writer(file, board.width, board.height) {
  }
  void writeSvgOutline();
  void writeGroup(Group const &);
  void writePad(Pad const &);
  void writeHole(Hole const &);
  void writeT(Rect rect, int orient);
public:
  Board board;
  SvgWriter writer;
};

void PIWHelper::writeT(Rect rect, int orient) {
  constexpr double pi = 3.14159265;
  Dim y = rect.center().y - rect.height * std::cos(pi*orient/180) / 2;
  Dim x = rect.center().x - rect.width * std::sin(pi*orient/180) / 2;
  writer << "<g transform=\"translate("
        << writer.coord(x) << "," << writer.coord(y) << "),"
        << "rotate(" << QString::number(180-orient) << ")\">\n";
  writer.fillRect(Rect(Point(Dim::fromMM(-1), Dim()),
                       Point(Dim::fromMM(1), Dim::fromMM(-.5))),
                  QColor(0, 0, 0));
  writer.fillRect(Rect(Point(Dim::fromMM(-.25), Dim()),
                       Point(Dim::fromMM(.25), Dim::fromMM(-2))),
                  QColor(0, 0, 0));
  writer << "</g>\n";
}
  
void PIWHelper::writeSvgOutline() {
  writer.drawRect(Rect(Point(), Point(board.width, board.height)),
                  QColor(255,0,0));
}

void PIWHelper::writeGroup(Group const &g) {
  for (int id: g.keys()) {
    Object const &obj(g.object(id));
    if (obj.isGroup())
      writeGroup(obj.asGroup());
    else if (obj.isPad())
      writePad(obj.asPad());
  }
}

static QColor pinColor(QString ref) {
  if (ref=="1" || ref.startsWith("1/") || ref.endsWith("/1"))
    return QColor(255, 0, 0);
  else
    return QColor(150, 150, 150);
} 

void PIWHelper::writePad(Pad const &pad) {
  if (pad.layer != Layer::Top)
    return; // don't do anything
  writer << "<g transform=\"translate("
         << writer.coord(pad.p.x) << "," << writer.coord(pad.p.y) << "),"
         << "rotate(" << pad.rota.toString() << ")\">\n";
  Dim x = pad.width/2;
  Dim y = pad.height/2;
  writer.fillRect(Rect(Point(-x,-y), Point(x,y)), pinColor(pad.ref));
  writer << "</g>\n";
}

void PIWHelper::writeHole(Hole const &hole) {
  writer.fillRing(hole.p, hole.od, hole.id, pinColor(hole.ref));
}

PNPImageWriter::PNPImageWriter() {
}

bool PNPImageWriter::write(Layout const &layout, PickNPlace const &pnp,
                           QString filename) {
  QFile file(filename);
  if (!file.open(QFile::WriteOnly | QFile::Text))
    return false;
  
  PIWHelper *d = new PIWHelper(&file, layout.board());
  d->writeSvgOutline();
  for (PNPLine l: pnp.placed()) {
    d->writer << "<!-- " << l.ref << "-->\n";
    d->writer.fillRect(l.bbox, QColor(200, 200, 200)); 
    Group const &g(layout.root().subByRef(l.ref));
    d->writeGroup(g);
    int tid = g.refTextId();
    qDebug() << l.ref << l.bbox << g.isEmpty() << g.ref << tid;
    if (tid>0) {
      Text const &t = layout.root().object(tid).asText();
      d->writer.drawText(t.text, t.p, QColor(100, 100, 1000),
                         Dim::fromInch(6.0/72), int(t.rota));
    }
    d->writeT(l.bbox, l.orient);
  }
  delete d;

  if (file.error() != QFile::NoError)
    return false;
  file.close();
  return true;
}
