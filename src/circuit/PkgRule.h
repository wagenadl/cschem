// PkgRule.h

#ifndef PACKAGE_H

#define PACKAGE_H

#include <QXmlStreamReader>
#include <QStringList>

class PkgRule {
public:
  QString report() const;
public:
  QString symbol;
  QStringList packages;
};

QXmlStreamWriter &operator<<(QXmlStreamWriter &, PkgRule const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, PkgRule &);


#endif
