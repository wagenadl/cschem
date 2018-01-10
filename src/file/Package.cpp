// Package.cpp

#include "Package.h"
#include "IDFactory.h"
#include <QPoint>

class PackageData: public QSharedData {
public:
  PackageData(): id(IDFactory::instance().newId()) { }
public:
  int id;
  QString notes;
  QString package;
  QString vendor;
  QString partno;
  QString mfgpart;
  QString manufacturer;  
};

Package::Package() {
  d = new PackageData();
}

Package::Package(Package const &o) {
  d = o.d;
}

Package &Package::operator=(Package const &o) {
  d = o.d;
  return *this;
}

Package::~Package() {
}


Package::Package(QXmlStreamReader &src): Package() {
  auto a = src.attributes();
  d->id = a.value("id").toInt();
  d->notes = a.value("notes").toString();
  d->package = a.value("package").toString();
  d->vendor = a.value("vendor").toString();
  d->partno = a.value("partno").toString();
  d->mfgpart = a.value("mfgpart").toString();
  d->manufacturer = a.value("manufacturer").toString();
  src.skipCurrentElement();
}

int Package::id() const {
  return d->id;
}

QString Package::package() const {
  return d->package;
}

QString Package::notes() const {
  return d->notes;
}

QString Package::vendor() const {
  return d->vendor;
}

QString Package::partno() const {
  return d->partno;
}

QString Package::mfgPart() const {
  return d->mfgpart;
}

QString Package::manufacturer() const {
  return d->manufacturer;
}

void Package::setPackage(QString s) {
  d.detach();
  d->package = s;
}

void Package::setNotes(QString s) {
  d.detach();
  d->notes = s;
}

void Package::setVendor(QString s) {
  d.detach();
  d->vendor = s;
}

void Package::setPartno(QString s) {
  d.detach();
  d->partno = s;
}

void Package::setMfgPart(QString s) {
  d.detach();
  d->mfgpart = s;
}

void Package::setManufacturer(QString s) {
  d.detach();
  d->manufacturer = s;
}

void Package::setId(int id) {
  d.detach();
  d->id = id;
}

QXmlStreamReader &operator>>(QXmlStreamReader &sr, Package &c) {
  c = Package(sr);
  return sr;
}
  
QXmlStreamWriter &operator<<(QXmlStreamWriter &sr, Package const &c) {
  sr.writeStartElement("package");
  sr.writeAttribute("id", QString::number(c.id()));
  if (!c.notes().isEmpty())
    sr.writeAttribute("notes", c.notes());
  if (!c.package().isEmpty())
    sr.writeAttribute("package", c.package());
  if (!c.vendor().isEmpty())
    sr.writeAttribute("vendor", c.vendor());
  if (!c.vendor().isEmpty())
    sr.writeAttribute("vendor", c.vendor());
  if (!c.partno().isEmpty())
    sr.writeAttribute("partno", c.partno());
  if (!c.mfgPart().isEmpty())
    sr.writeAttribute("mfgpart", c.mfgPart());
  if (!c.manufacturer().isEmpty())
    sr.writeAttribute("manufacturer", c.manufacturer());
  sr.writeEndElement();
  return sr;
};

