// Package.cpp

#include "Package.h"

#include  <QDebug>

static QString pinmapAsString(QMap<QString, int> const &pinmap) {
  QStringList pins;  
  for (QString pn: pinmap.keys())
    pins << QString("%1=%2").arg(pn).arg(pinmap[pn]);
  return pins.join(" ");
}

QXmlStreamReader &operator>>(QXmlStreamReader &sr, Package &c) {
  c = Package(); // reset
  auto const &a = sr.attributes();

  c.name = a.value("name").toString();

  c.pcb = a.value("pcb").toString();

  QString map = a.value("map").toString();
  if (map.contains(" ")) {
    // map of type "name1=number1 name2=number2 ..."
    int k = 0;
    QStringList lst = map.split(" ");
    for (QString p: lst) {
      QStringList bits = p.split("=");
      if (bits.size()==1) {
	c.pinmap[bits[0]] = ++k;
      } else if (bits.size()==2) {
	k = bits[1].toInt();
	c.pinmap[bits[0]] = k;
      } else {
	qDebug() << "Cannot interpret" << p << "as a pin map element";
      }
    }
  } else if (!map.isEmpty()) {
    for (int k=0; k<map.size(); k++)
      c.pinmap[map.mid(k,1)] = k+1;
  }
  
  sr.skipCurrentElement();
  return sr;
}

QXmlStreamWriter &operator<<(QXmlStreamWriter &sw, Package const &c) {
  sw.writeStartElement("package");
  sw.writeAttribute("name", c.name);
  sw.writeAttribute("pcb", c.pcb);
  QString map = pinmapAsString(c.pinmap);
  if (!map.isEmpty())
    sw.writeAttribute("map", map);
  sw.writeEndElement();
  return sw;
};
  
QString Package::report() const {
  QStringList pins;
  return QString("%1=%2: [%3]")
    .arg(name).arg(pcb)
    .arg(pinmapAsString(pinmap));
}
