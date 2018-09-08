// PkgRule.h

#ifndef PKGRULE_H

#define PKGRULE_H

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
