// qschem.cpp

#include "svg/SymbolLibrary.h"
#include "file/FileIO.h"
#include "circuit/Schem.h"
#include <QApplication>
#include <QGraphicsView>
#include "ui/Scene.h"
#include "ui/MainWindow.h"
#include "circuit/Net.h"
#include <QDebug>
#include "svg/Paths.h"
#include <QDir>

void ensureSymbolLibrary() {
  QDir userlib(Paths::userSymbolRoot());
  if (!userlib.exists())
    userlib.mkpath(".");

  if (!userlib.exists("System")) {
    QString sysloc(Paths::systemSymbolRoot());
    if (!sysloc.isEmpty())
      QFile(sysloc).link(userlib.absoluteFilePath("System"));
  }
}

int main(int argc, char **argv) {
  QApplication app(argc, argv);

  ensureSymbolLibrary();
  
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
