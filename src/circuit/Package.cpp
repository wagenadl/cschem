// Package.cpp

#include "Package.h"


class PackageData: public QSharedData {
public:
  PackageData() {
    reset();
  }
  void reset() {
    name = "";
    pcb = "";
    pinmap.clear();
  }
  QStringList pinmapAsStringList() const {
    QStringList pins;  
    for (QString pn: pinmap.keys())
      pins << QString("%1=%2").arg(pn).arg(pinmap[pn]);
    return pins;
  }
public:
  QString name;
  QString pcb;
  QMap<QString, int> pinmap;
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

void Package::readAttributes(QXmlStreamReader &src) {
  auto a = src.attributes();
  d->id = a.value("id").toInt();
  d->position = QPoint(a.value("x").toInt(), a.value("y").toInt());
  d->layer = layerFromAbbreviation(a.value("layer").toString());
  d->subtype = a.value("type").toString();
  d->value = a.value("value").toString();
  d->valuePos = QPoint(a.value("valx").toInt(), a.value("valy").toInt());
  d->valueVis = a.value("valvis").toInt() > 0;
  d->name = a.value("name").toString();
  d->namePos = QPoint(a.value("namex").toInt(), a.value("namey").toInt());
  d->nameVis = a.value("namevis").toInt() > 0;
  d->rotation = a.value("rotation").toInt();
  d->flip = a.value("flip").toInt() ? true : false;
  d->info.vendor = a.value("vendor").toString();
  d->info.partno = a.value("partno").toString();
  d->info.package = a.value("package").toString();
  d->info.notes = a.value("notes").toString();
}  

QString Package::name() const {
  return d->name;
}

QString Package::pcb() const {
  return d->pcb;
}

QMap<QString, int> Package::pinmap() const {
  return d->pinmap;
}

QXmlStreamReader &operator>>(QXmlStreamReader &sr, Package &c) {
  c.d->reset();
  auto a = src.attributes();
  d->name = a.value("name").toString();
  d->pcb = a.value("pcb").toString();
  QString map = a.value("map").toString();
  if (map.contains(" ")) {
    // map of type "name1=number1 name2=number2 ..."
    QStringList lst = map.split(" ");
    for (QString p: lst) {
      QStringList bits = p.split("=");
      if (bits.size()==2) {
	d->map[bits[0]] = bits[1].toInt();
      } else {
	qDebug() << "Cannot interpret" << p << "as a pin map element";
      }
    }
  } else if (!map.isEmpty()) {
    for (int k=0; k<map.size(); k++)
      d->map[map.mid(k,1)] = k+1;
  }

  sr.skipCurrentPackage();
  return sr;
}

QXmlStreamWriter &operator<<(QXmlStreamWriter &sw, Package const &c) {
  sw.writeStartPackage("package");
  sw.writeAttribute("name", c.name());
  sw.writeAttribute("pcb", c.pcb());
  QStringList map = c.d->pinmapTostring();
  if (!map.isEmpty())
    sw.writeAttribute("map", map.join(" ");
  sw.writeEndPackage();
  return sw;
};
  
QString Package::report() const {
  QStringList pins;
  for (QString pn: d->map.keys())
  return QString("%1=%2: [%3]")
    .arg(name()).arg(pcb())
    .arg(d->pinmapTostring().join(" "));
}
