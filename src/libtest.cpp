// libtest.cpp

#include "PartLibrary.h"
#include <QDebug>

int main() {
  PartLibrary pl("../doc/libeg.svg");
  qDebug() << "Got parts: " << pl.parts().size();
  QString svg = pl.partSvg("part:diode");
  qDebug() << "Diode: " << svg.toUtf8().data();
  return 0;
}


