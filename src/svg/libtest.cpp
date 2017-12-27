// libtest.cpp

#include "PartLibrary.h"
#include <QDebug>

int main() {
  PartLibrary pl("../doc/libeg.svg");
  qDebug() << "Got parts: " << pl.partNames().size();
  QString svg = pl.partSvg("part:diode");
  QFile f("../doc/diodee.svg");
  f.open(QFile::WriteOnly);
  f.write(svg.toUtf8());
  return 0;
}


