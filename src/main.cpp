// main.cpp

#include "data/Trace.h"
#include "ui/MainWindow.h"
#include "data/Object.h"
#include <QApplication>
#include <QFile>

int main(int argc, char **argv) {
  QApplication app(argc, argv);
  Trace trc;
  trc.layer = Layer::Bottom;
  trc.width = Dim::fromMils(15);
  trc.p1 = Point(Dim::fromMils(100), Dim::fromMils(200));
  trc.p2 = Point(Dim::fromMils(900), Dim::fromMils(200));
  //  qDebug() << trc;

  QFile file("doc/test.xml");
  QFile out("/tmp/test.xml");
  if (!out.open(QFile::WriteOnly)) {
    qDebug() << "Failed to write";
    return 1;
  }
  if (!file.open(QFile::ReadOnly)) {
    qDebug() << "Failed to read";
    return 1;
  }

  { QXmlStreamWriter sw(&out);
  sw.setAutoFormatting(true);
  sw.setAutoFormattingIndent(2);
  sw.writeStartDocument("1.0", false);
  sw.writeStartElement("cpcb");
  
  QXmlStreamReader s(&file);
  while (!s.atEnd()) {
    s.readNext();
    if (s.isStartElement() && s.name() != "cpcb") {
      Object o;
      s >> o;
      sw << o;
      qDebug() << o;
    }
  }

  sw.writeEndElement();
  sw.writeEndDocument();
  }
  
  MainWindow mw;
  mw.show();
  return app.exec();
}

