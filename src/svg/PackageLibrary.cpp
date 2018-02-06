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
  qDebug() << "drawing" << name << fn;
  if (!drawings().contains(fn))
    drawings()[fn] = PackageDrawing(fn);
  return drawings()[fn];
}

QStringList PackageLibrary::compatiblePackages(Symbol const &) const {
  qDebug() << "Compatible packages (based on pin count) NYI";
  return QStringList();
}
