// qschem.cpp

#include "svg/PartLibrary.h"
#include "file/FileIO.h"
#include "file/Schem.h"
#include <QApplication>
#include <QGraphicsView>
#include "ui/Scene.h"
#include "ui/Editor.h"

int main(int argc, char **argv) {
  PartLibrary lib(":parts.svg");
  Schem s(FileIO::loadSchematic("../doc/example.xml"));
  FileIO::saveSchematic("/tmp/eg.xml", s);
  QApplication app(argc, argv);
  Editor editor(&lib, &s);
  editor.show();
  return app.exec();
}
