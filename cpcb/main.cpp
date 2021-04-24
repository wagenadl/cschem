// main.cpp

#include "data/Trace.h"
#include "ui/MainWindow.h"
#include "data/Object.h"
#include <QApplication>
#include <QFile>
#include <QScreen>
#include <QDir>
#include "data/Paths.h"
#include <QSysInfo>

void ensureOutlineLibrary() {
  QDir recentdir(Paths::recentSymbolsLocation());
  if (!recentdir.exists())
    recentdir.mkpath(".");
  
  QDir userlib(Paths::userComponentRoot());
  if (!userlib.exists())
    userlib.mkpath(".");

  if (!userlib.exists("System")) {
    QString sysloc(Paths::systemComponentRoot());
    if (!sysloc.isEmpty()) {
      QString linkname = userlib.absoluteFilePath("System");
      if (QSysInfo::productType() == "windows")
          linkname += ".lnk";
      QFile(sysloc).link(linkname);
    }
  }
}
  

int main(int argc, char **argv) {
  QApplication app(argc, argv);
  app.setApplicationName("cschem");
  app.setApplicationDisplayName("CPCB");
  Paths::setExecutablePath(argv[0]);
  
  app.setStyleSheet("QToolButton:!checked { border: none; }\n"
                    "QToolButton:checked { border: 3px inset #666666; border-radius: 2; background-color: white;}\n");

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

