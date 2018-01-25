// Package.h

#ifndef PACKAGE_H

#define PACKAGE_H

#include <QSharedData>
#include <QXmlStreamReader>
#include  <QString>
#include <QMap>

class Package {
public:
  Package();
  Package(Package const &);
  Package &operator=(Package const &);
  ~Package();
  QString report() const;
public:
  QString name() const; // e.g., "r3" or "dip8"
  QString pcb() const; // e.g., "ACY300" or  "DIP8"
  QMap<QString, int> pinmap() const;
private:
  friend QXmlStreamWriter &operator<<(QXmlStreamWriter &, Package const &);
  friend QXmlStreamReader &operator>>(QXmlStreamReader &, Package &);
private:
  QSharedDataPointer<class PackageData> d;
};

QXmlStreamWriter &operator<<(QXmlStreamWriter &, Package const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Package &);


#endif
