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
#include <QFileOpenEvent>

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
    qDebug() << "sysloc" << sysloc;
    if (!sysloc.isEmpty()) {
      QString linkname = userlib.absoluteFilePath("System");
      if (QSysInfo::productType() == "windows")
          linkname += ".lnk";
      qDebug() << "linkname" << linkname;
      QFile(sysloc).link(linkname);
    }
  }
}
  
class CPCBApplication: public QApplication {
public:
  CPCBApplication(int &argc, char **argv): QApplication(argc, argv) {
    setOrganizationName("cschem");
    setOrganizationDomain("danielwagenaar.net");
    setApplicationName("cpcb");
    setApplicationDisplayName("CPCB");
  }

  bool event(QEvent *evt) override {
    if (evt->type() == QEvent::FileOpen) {
      qDebug() << "FileOpen event";
      QFileOpenEvent *evt1 = static_cast<QFileOpenEvent *>(evt);
      const QUrl url = evt1->url();
      if (url.isLocalFile()) {
        qDebug() << "Is local file";
        MainWindow *mw = new MainWindow;
        if (!mw->open(url.toLocalFile())) {
          mw->deleteLater();
          qDebug() << "Failed to load";
        }
      }
    }
    return QApplication::event(evt);
  }
};


int main(int argc, char **argv) {
  CPCBApplication app(argc, argv);
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

