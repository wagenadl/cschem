// PackageLibrary.h

#ifndef PACKAGELIBRARY_H

#define PACKAGELIBRARY_H

class PackageLibrary {
public:
  PackageLibrary(QString fppath, Packaging const &);
  PackageLibrary();
  void merge(QString fppath, Packaging const &);
  ~PackageLibrary();
};

#endif
