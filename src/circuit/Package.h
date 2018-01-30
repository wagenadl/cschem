// Package.h

#ifndef PACKAGE_H

#define PACKAGE_H

#include <QXmlStreamReader>
#include <QString>
#include <QMap>

class Package {
public:
  QString report() const;
public:
  QString name;
  QString pcb;
  QMap<QString, int> pinmap;
};

QXmlStreamWriter &operator<<(QXmlStreamWriter &, Package const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Package &);


#endif
