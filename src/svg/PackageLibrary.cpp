// PackageLibrary.cpp

#include "PackageLibrary.h"
#include "PackageDrawing.h"
#include "Symbol.h"

#include <QDebug>
#include <QDir>

QString PackageLibrary::defaultPath() {
  return "/usr/share/pcb/pcblib-newlib/geda";
}

static QMap<QString, PackageDrawing> &drawings() {
  static QMap<QString, PackageDrawing> drw;
  return drw;
} // maps from full pathname

PackageLibrary::PackageLibrary() {
  fppath = defaultPath();
}

PackageLibrary::PackageLibrary(Packaging const &pkg, QString fpp):
  Packaging(pkg) {
  if (fpp.isEmpty())
    fppath = defaultPath();
  else
    fppath = fpp;
}

void PackageLibrary::merge(Packaging const &pkg) {
  *this += pkg;
}

void PackageLibrary::setPath(QString p) {
  fppath = p;
}

PackageLibrary const &PackageLibrary::defaultPackages() {
  static PackageLibrary pkgs;
  static bool ready = false;
  if (!ready) {
    ready = true;
    QFile file(":packages.xml");
    if (file.open(QFile::ReadOnly)) {
      QXmlStreamReader sr(&file);
      sr >> pkgs;
    }
  }
  return pkgs;
}

PackageDrawing const &PackageLibrary::drawing(QString name) const {
  static PackageDrawing nil;
  if (!packages.contains(name))
    return nil;
  QString pcb = packages[name].pcb;
  QDir dir(fppath);
  QString fn(dir.absoluteFilePath(pcb + ".fp"));
  if (!drawings().contains(fn))
    drawings()[fn] = PackageDrawing(fn);
  return drawings()[fn];
}

QStringList PackageLibrary::compatiblePackages(Symbol const &sym) const {
  int nPins = sym.pinNames().size();
  QString name = sym.name();
  QStringList res;
  for (QString const &name: packages.keys()) {
    PackageDrawing const &drw = drawing(name);
    if (drw.pins().size()==nPins)
      res << name;
  }
  return res;
}
