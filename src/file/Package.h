// Package.h

#ifndef PACKAGE_H

#define PACKAGE_H

#include <QSharedData>
#include <QXmlStreamReader>

class Package {
public:
  Package();
  Package(Package const &);
  Package(QXmlStreamReader &src);
  Package &operator=(Package const &);
  ~Package();
public:
  int id() const;
  QString package() const;
  QString vendor() const;
  QString partno() const;
  QString mfgPart() const;
  QString manufacturer() const;
public:
  void setId(int);
  void setPackage(QString);
  void setVendor(QString);
  void setPartno(QString);
  void setMfgPart(QString);
  void setManufacturer(QString);
private:
  QSharedDataPointer<class PackageData> d;
};

QXmlStreamWriter &operator<<(QXmlStreamWriter &, Package const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Package &);


#endif
