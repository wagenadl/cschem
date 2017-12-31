// qschem.cpp

#include "svg/PartLibrary.h"
#include "file/FileIO.h"
#include "file/Schem.h"
#include <QApplication>
#include <QGraphicsView>
#include "ui/Scene.h"
#include "ui/Editor.h"
#include "file/Net.h"
#include <QDebug>

int main(int argc, char **argv) {
  PartLibrary lib(":parts.svg");
  Schem s(FileIO::loadSchematic("../doc/example.xml"));
  Net net(s.circuit(), 1, "1");
  qDebug() << "Net report:";
  for (auto p: net.pins()) {
    Element const &elt = s.circuit().element(p.element());
    qDebug() << "Pin" << elt.id() << elt.tag() << elt.name() << ":" << p.pin();
  }
  for (auto c: net.connections()) {
    Connection const &con = s.circuit().connection(c);
    qDebug() << "(Connection" << con.id() << ")";
  }
  
  QApplication app(argc, argv);
  Editor editor(&lib, &s);
  editor.show();
  editor.resize(editor.size()*2);
  return app.exec();
}
