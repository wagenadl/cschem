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
  //  qDebug() << "userlib" << userlib.absolutePath();
  if (!userlib.exists())
    userlib.mkpath(".");

  if (!userlib.exists("System")) {
    QString sysloc(Paths::systemSymbolRoot());
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

int main(int argc, char **argv) {
  QApplication app(argc, argv);
  app.setOrganizationName("cschem");
  app.setOrganizationDomain("danielwagenaar.net");
  app.setApplicationName("cschem");
  app.setApplicationDisplayName("CSchem");
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
  // cli.addOption(cli_png);
  // cli.addOption(cli_res);
  cli.addOption(cli_ofn);

  cli.process(app);
  QStringList args = cli.positionalArguments();

  bool ok = true;

  if (cli.isSet("export-svg"))
    for (QString fn: args)
      ok = exportSvg(cli.value("o"), fn) && ok;

//  if (cli.isSet("export-png"))
//    for (QString fn: args)
//      ok = exportPng(cli.value("o"), cli.value("resolution").toInt(), fn) && ok;

  if (cli.isSet("export-svg")) // || cli.isSet("export-png"))
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
