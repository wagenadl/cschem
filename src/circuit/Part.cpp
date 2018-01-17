// Part.cpp

#include "Part.h"
#include "IDFactory.h"
#include <QPoint>

class PartData: public QSharedData {
public:
  PartData(): id(IDFactory::instance().newId()) {
    reset();
  }
  void reset() {
    value = name = "";
    vendor = partno = "";
    package = notes = "";
  }
public:
  int id;
  QString name;
  QString value;
  QString vendor;
  QString partno;
  QString package;
  QString notes;
};

Part::Part() {
  d = new PartData();
}

Part::Part(Part const &o) {
  d = o.d;
}

Part &Part::operator=(Part const &o) {
  d = o.d;
  return *this;
}

Part::~Part() {
}

void Part::readAttributes(QXmlStreamReader &src) {
  auto a = src.attributes();
  d->id = a.value("id").toInt();
  d->name = a.value("name").toString();
  d->value = a.value("value").toString();
  d->vendor = a.value("vendor").toString();
  d->partno = a.value("partno").toString();
  d->package = a.value("package").toString();
  d->notes = a.value("notes").toString();
}  

QString Part::value() const {
  return d->value;
}

QString Part::name() const {
  return d->name;
}

QString Part::vendor() const {
  return d->vendor;
}

QString Part::partno() const {
  return d->partno;
}

QString Part::package() const {
  return d->package;
}

QString Part::notes() const {
  return d->notes;
}

int Part::id() const {
  return d->id;
}
  
void Part::setValue(QString v) {
  d.detach();
  d->value = v;
}

void Part::setName(QString n) {
  d.detach();
  d->name = n;
}

void Part::setVendor(QString n) {
  d.detach();
  d->vendor = n;
}

void Part::setPartno(QString n) {
  d.detach();
  d->partno = n;
}

void Part::setPackage(QString n) {
  d.detach();
  d->package = n;
}

void Part::setNotes(QString n) {
  d.detach();
  d->notes = n;
}

void Part::setId(int id) {
  d.detach();
  d->id = id;
}

bool Part::isValid() const {
  return !d->name.isEmpty();
}

QXmlStreamReader &operator>>(QXmlStreamReader &sr, Part &c) {
  c.d->reset();
  c.readAttributes(sr);
  sr.skipCurrentElement();
  return sr;
}

QXmlStreamWriter &operator<<(QXmlStreamWriter &sw, Part const &c) {
  sw.writeStartElement("part");
  c.writeAttributes(sw);
  sw.writeEndElement();
  return sw;
}

void Part::writeAttributes(QXmlStreamWriter &sw) const {
  sw.writeAttribute("id", QString::number(id()));
  if (!value().isEmpty()) 
    sw.writeAttribute("value" , value());
  if (!name().isEmpty()) 
    sw.writeAttribute("name", name());
  if (!vendor().isEmpty())
    sw.writeAttribute("vendor", vendor());
  if (!partno().isEmpty())
    sw.writeAttribute("partno", partno());
  if (!package().isEmpty())
    sw.writeAttribute("package", package());
  if (!notes().isEmpty())
    sw.writeAttribute("notes", notes());
}  

QString Part::report() const {
  return QString("%1: %2 %3 in %4");
    .arg(id())
       .arg(name()).arg(value())
       .arg(package());
}
