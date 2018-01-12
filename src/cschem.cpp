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
  QList<MainWindow *> mws;
  if (argc == 1) {
    mws << new MainWindow;
  } else {
    for (int i=1; i<argc; i++) {
      MainWindow *mw = new MainWindow;
      mw->load(argv[i]);
      mws << mw;
    }
  }
  for (auto *mw: mws)
    mw->show();

  return app.exec();
}
