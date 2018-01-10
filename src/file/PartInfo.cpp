// PartInfo.cpp

#include "PartInfo.h"
#include "IDFactory.h"
#include <QPoint>

class PartInfoData: public QSharedData {
public:
  PartInfoData(): id(IDFactory::instance().newId()) { }
public:
  int id;
  QString notes;
  QString package;
  QString vendor;
  QString partno;
  QString mfgpart;
  QString manufacturer;  
};

PartInfo::PartInfo() {
  d = new PartInfoData();
}

PartInfo::PartInfo(PartInfo const &o) {
  d = o.d;
}

PartInfo &PartInfo::operator=(PartInfo const &o) {
  d = o.d;
  return *this;
}

PartInfo::~PartInfo() {
}


PartInfo::PartInfo(QXmlStreamReader &src): PartInfo() {
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

int PartInfo::id() const {
  return d->id;
}

QString PartInfo::package() const {
  return d->package;
}

QString PartInfo::notes() const {
  return d->notes;
}

QString PartInfo::vendor() const {
  return d->vendor;
}

QString PartInfo::partno() const {
  return d->partno;
}

QString PartInfo::mfgPart() const {
  return d->mfgpart;
}

QString PartInfo::manufacturer() const {
  return d->manufacturer;
}

void PartInfo::setPackage(QString s) {
  d.detach();
  d->package = s;
}

void PartInfo::setNotes(QString s) {
  d.detach();
  d->notes = s;
}

void PartInfo::setVendor(QString s) {
  d.detach();
  d->vendor = s;
}

void PartInfo::setPartno(QString s) {
  d.detach();
  d->partno = s;
}

void PartInfo::setMfgPart(QString s) {
  d.detach();
  d->mfgpart = s;
}

void PartInfo::setManufacturer(QString s) {
  d.detach();
  d->manufacturer = s;
}

void PartInfo::setId(int id) {
  d.detach();
  d->id = id;
}

QXmlStreamReader &operator>>(QXmlStreamReader &sr, PartInfo &c) {
  c = PartInfo(sr);
  return sr;
}
  
QXmlStreamWriter &operator<<(QXmlStreamWriter &sr, PartInfo const &c) {
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

