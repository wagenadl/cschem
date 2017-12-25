// PartLibrary.h

#ifndef PARTLIBRARY_H

#define PARTLIBRARY_H

#include "Part.h"

class PartLibrary {
public:
  PartLibrary(QString fn);
  ~PartLibrary();
  QMap<QString, Part const *> parts() const { return parts_; }
  QString partSvg(QString name);
private:
  void scanParts(XmlElement const &src);
  void getBBoxes(QString fn);
private:
  QList<Part> partslist_;
  QMap<QString, Part const *> parts_;
  XmlElement svg_;
};

#endif
