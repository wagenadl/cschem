// qschem.cpp

#include "svg/PartLibrary.h"
#include "file/FileIO.h"
#include "file/Schem.h"
#include <QApplication>
#include <QGraphicsView>
#include "ui/Scene.h"
#include "ui/MainWindow.h"
#include "file/Net.h"
#include <QDebug>

int main(int argc, char **argv) {
  QApplication app(argc, argv);
  MainWindow mw;
  mw.load("../doc/example.xml");
  mw.show();
  mw.resize(mw.size()*2);
  return app.exec();
}
