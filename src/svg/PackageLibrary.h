// PackageLibrary.h

#ifndef PACKAGELIBRARY_H

#define PACKAGELIBRARY_H

#include "circuit/Packaging.h"

class PackageLibrary: public Packaging {
public:
  PackageLibrary();
  explicit PackageLibrary(Packaging const &, QString fppath="");
  void merge(Packaging const &);
  void setPath(QString);
  class PackageDrawing const &drawing(QString name); // our name, not pcb's
public:
  static PackageLibrary const &defaultPackages();
  static QString defaultPath();
private:
  QString fppath;
};

#endif
