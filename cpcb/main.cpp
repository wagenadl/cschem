// main.cpp

#include "data/Trace.h"
#include "ui/MainWindow.h"
#include "data/Object.h"
#include <QApplication>
#include <QFile>
#include <QScreen>
#include <QDir>
#include "data/Paths.h"

void ensureOutlineLibrary() {
  QDir recentdir(Paths::recentSymbolsLocation());
  if (!recentdir.exists())
    recentdir.mkpath(".");
  
  QDir userlib(Paths::userComponentRoot());
  if (!userlib.exists())
    userlib.mkpath(".");

  if (!userlib.exists("System")) {
    QString sysloc(Paths::systemComponentRoot());
    if (!sysloc.isEmpty())
      QFile(sysloc).link(userlib.absoluteFilePath("System"));
  }
}
  

int main(int argc, char **argv) {
  QApplication app(argc, argv);

  ensureOutlineLibrary();
  
  QStringList args = app.arguments();
  
  MainWindow mw;

  if (args.size()>=2)
    mw.open(args.last());  
  QSize avg = app.primaryScreen()->availableSize();
  mw.resize(3*avg.width()/4, 3*avg.height()/4);
  mw.show();
  return app.exec();
}

