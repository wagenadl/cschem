// Packaging.h

#ifndef PACKAGING_H

#define PACKAGING_H

#include "Package.h"
#include "PkgRule.h"

#include <QXmlStreamReader>
#include <QMap>

class Packaging {
public:
  QString report() const;
  Packaging &operator+=(Packaging const &o);
public:
  QMap<QString, Package> packages;
  QMap<QString, PkgRule> rules;
};

QXmlStreamWriter &operator<<(QXmlStreamWriter &, Packaging const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Packaging &);

#endif
