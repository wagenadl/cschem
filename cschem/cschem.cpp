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
#include <QCommandLineParser>
#include "svg/SvgExporter.h"
#include <QFileOpenEvent>

bool exportSvg(QString ofn, QString ifn) {
  Schem s = FileIO::loadSchematic(ifn);
  if (s.isEmpty()) {
    qDebug() << "Failed to load schematic" << ifn;
    return false;
  }
  SvgExporter xp(s);
  if (ofn.isEmpty())
    ofn = ifn.replace(".cschem", "") + ".svg";
  if (!xp.exportSvg(ofn)) {
    qDebug() << "Failed to export svg" << ofn;
    return false;
  }    
  return true;
}

bool exportPng(QString ofn, int res, QString ifn) {
  qDebug() << "exportpng" << ifn << ofn << res;
  return false;
}

void ensureSymbolLibrary() {
  QDir userlib(Paths::userSymbolRoot());
  qDebug() << "userlib" << userlib.absolutePath();
  if (!userlib.exists())
    userlib.mkpath(".");

  QString sysloc(Paths::systemSymbolRoot());
  qDebug() << "sysloc" << sysloc;
  if (!userlib.exists("System")) {
    if (!sysloc.isEmpty()) {
      QString linkname = userlib.absoluteFilePath("System");
      if (QSysInfo::productType() == "windows")
          linkname += ".lnk";
      qDebug() << "linkname" << linkname;
      QFile(sysloc).link(linkname);
    }
  }
}

class CSchemApplication: public QApplication {
public:
  CSchemApplication(int &argc, char **argv): QApplication(argc, argv) {
    setOrganizationName("cschem");
    setOrganizationDomain("danielwagenaar.net");
    setApplicationName("cschem");
    setApplicationDisplayName("CSchem");
  }

  bool event(QEvent *evt) override {
    if (evt->type() == QEvent::FileOpen) {
      /* This is how MacOS opens double-clicked files.
         It might be good to see if any "empty" main windows exist
      */
      qDebug() << "FileOpen event";
      QFileOpenEvent *evt1 = static_cast<QFileOpenEvent *>(evt);
      const QUrl url = evt1->url();
      if (url.isLocalFile()) {
        qDebug() << "Is local file";
        MainWindow *mw = new MainWindow;
        if (mw->load(url.toLocalFile())) {
	  mw->show();
        } else {
          mw->deleteLater();
          qDebug() << "Failed to load";
        }
      }
    }
    return QApplication::event(evt);
  }
};

int main(int argc, char **argv) {
  CSchemApplication app(argc, argv);
  Paths::setExecutablePath(argv[0]);

  ensureSymbolLibrary();

  QCommandLineOption cli_svg("export-svg", "Convert schematic to SVG.");
  QCommandLineOption cli_png("export-png", "Convert schematic to PNG.");
  QCommandLineOption cli_res(QStringList{"resolution", "r"}, "Resolution for PNG export (pix/inch).", "ppi", "300");
  QCommandLineOption cli_ofn(QStringList{"o"}, "Output filename for export.", "file", "");
  QCommandLineParser cli;
  
  cli.setApplicationDescription("\n"
    "CSchem — Electronic circuit design and PCB layout\n"
    "\n"
    "More information is at https://github.com/wagenadl/cschem");
  cli.addHelpOption();
  cli.addVersionOption();
  cli.addPositionalArgument("filename", "Specify file to open", "[filename.cschem]");
  cli.addOption(cli_svg);
  cli.addOption(cli_ofn);

  cli.process(app);
  QStringList args = cli.positionalArguments();

  bool ok = true;

  if (cli.isSet("export-svg"))
    for (QString fn: args)
      ok = exportSvg(cli.value("o"), fn) && ok;

  if (cli.isSet("export-svg"))
    return ok ? 0 : 1;
  
  QList<MainWindow *> mws;
  for (QString fn: args) {
    MainWindow *mw = new MainWindow;
    if (!mw->load(fn))
      return 1;
    mws << mw;
  }

  if (mws.isEmpty()) 
    mws << new MainWindow; // new file
  
  for (auto *mw: mws)
    mw->show();

  return app.exec();
}
