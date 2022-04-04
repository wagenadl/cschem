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
#include <QSysInfo>

void ensureSymbolLibrary() {
  QDir userlib(Paths::userSymbolRoot());
  qDebug() << "userlib" << userlib.absolutePath();
  if (!userlib.exists())
    userlib.mkpath(".");

  if (!userlib.exists("System")) {
    QString sysloc(Paths::systemSymbolRoot());
    qDebug() << "syslib" << sysloc;
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
  app.setOrganizationName("Daniel Wagenaar");
  app.setOrganizationDomain("danielwagenaar.net");
  app.setApplicationName("cschem");
  app.setApplicationDisplayName("CSchem");
  Paths::setExecutablePath(argv[0]);

  ensureSymbolLibrary();
  
  QList<MainWindow *> mws;
  if (argc == 1) {
    mws << new MainWindow;
  } else {
    bool ok = false;
    for (int i=1; i<argc; i++) {
      MainWindow *mw = new MainWindow;
      if (mw->load(argv[i]))
	ok = true;
      mws << mw;
    }
    if (!ok)
      return 1;
  }
  for (auto *mw: mws)
    mw->show();

  return app.exec();
}
