// qschem.cpp

#include "svg/PartLibrary.h"
#include "file/FileIO.h"
#include "file/Schem.h"
#include <QApplication>
#include <QGraphicsView>
#include "ui/Scene.h"

int main(int argc, char **argv) {
  PartLibrary lib(":parts.svg");
  Schem s(FileIO::loadSchematic("../doc/example.xml"));
  FileIO::saveSchematic("/tmp/eg.xml", s);
  QApplication app(argc, argv);
  QGraphicsView view;
  view.setInteractive(true);
  view.setDragMode(QGraphicsView::RubberBandDrag);
  Scene scene(&lib);
  scene.setCircuit(s.circuit());
  view.setScene(&scene);
  view.scale(4, 4);
  view.show();
  return app.exec();
}
