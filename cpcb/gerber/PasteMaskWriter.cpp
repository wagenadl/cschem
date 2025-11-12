// PasteMaskWriter.cpp

#include "PasteMaskWriter.h"
#include "SvgWriter.h"
#include "data/Layout.h"
#include "data/Object.h"
#include <QFile>
#include <QTextStream>

class PMWHelper {
public:
  PMWHelper(QFile *file, Board const &board, Dim shrink):
    board(board),
    writer(file, board.width, board.height),
    shrinkage(shrink) {
  }
  void writeSvgOutline();
  void writeGroup(Group const &);
  void writePad(Pad const &);
public:
  Board board;
  SvgWriter writer;
  Dim shrinkage;
};
  
void PMWHelper::writeSvgOutline() {
  writer.drawRect(Rect(Point(), Point(board.width, board.height)),
                  QColor(255,0,0));
}

void PMWHelper::writeGroup(Group const &g) {
  for (int id: g.keys()) {
    Object const &obj(g.object(id));
    if (obj.isGroup())
      writeGroup(obj.asGroup());
    else if (obj.isPad())
      writePad(obj.asPad());
  }
}

void PMWHelper::writePad(Pad const &pad) {
  if (pad.layer != Layer::Top)
    return; // don't do anything
  writer.startGroup(pad.p, pad.rota.degrees());
  Dim x0 = -pad.width/2 + shrinkage;
  Dim y0 = -pad.height/2 + shrinkage;
  Dim x1 = pad.width/2 - shrinkage;
  Dim y1 = pad.height/2 - shrinkage;
  writer.drawRect(Rect(Point(x0,y0), Point(x1,y1)),
                  QColor(0,0,0), Dim::fromInch(.25/96));
  writer.endGroup();
}

PasteMaskWriter::PasteMaskWriter(): shrinkage(Dim::fromInch(0.005)) {
}

void PasteMaskWriter::setShrinkage(Dim s) {
  shrinkage = s;
}

bool PasteMaskWriter::write(Layout const &layout, QString filename) {
  QFile file(filename);
  if (!file.open(QFile::WriteOnly | QFile::Text))
    return false;
  
  PMWHelper *d = new PMWHelper(&file, layout.board(), shrinkage);
  d->writeSvgOutline();
  d->writeGroup(layout.root());
  delete d;

  if (file.error() != QFile::NoError)
    return false;
  file.close();
  return true;
}
