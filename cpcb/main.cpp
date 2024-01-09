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

  // qDebug() << "ensure" << userlib;

  if (!userlib.exists("System")) {
    QString sysloc(Paths::systemComponentRoot());
    qDebug() << "..." << sysloc;
    if (!sysloc.isEmpty()) {
      QString linkname = userlib.absoluteFilePath("System");
      qDebug() << "..." << linkname;
      if (QSysInfo::productType() == "windows")
          linkname += ".lnk";
      QFile(sysloc).link(linkname);
    }
  }
}
  

int main(int argc, char **argv) {
  QApplication app(argc, argv);
  app.setOrganizationName("cschem");
  app.setOrganizationDomain("danielwagenaar.net");
  app.setApplicationName("cpcb");
  app.setApplicationDisplayName("CPCB");
  Paths::setExecutablePath(argv[0]);
  
  app.setStyleSheet("QToolButton:!checked { border: none; }\n"
                    "QToolButton:checked { border: 3px inset #666666;"
                    "  border-radius: 2; background-color: white;}\n"
                    //"QWidget { font-family: Verdana, Helvetica, Sans;"
                    //"  font-size: 10pt; }\n"
                    );
  QFont font = app.font();
  font.setPointSizeF(11.0);
  //qDebug() << "CPCB FONT" << font;
  app.setFont(font);

  ensureOutlineLibrary();
  
  QStringList args = app.arguments();

  QList<MainWindow *> mws;
  if (argc==1) {
    mws << new MainWindow;
  } else {
    bool ok = false;
    for (int i=1; i<argc; i++) {
      MainWindow *mw = new MainWindow;
      if (mw->open(argv[i]))
	ok = true;
      mws << mw;
      if (!ok)
	return 1;
    }
  }

  for (auto *mw: mws)
    mw->show();

  return app.exec();
}

