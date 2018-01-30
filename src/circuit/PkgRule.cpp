// PkgRule.cpp

#include "PkgRule.h"

#include  <QDebug>

QXmlStreamReader &operator>>(QXmlStreamReader &sr, PkgRule &c) {
  c = PkgRule(); // reset
  auto const &a = sr.attributes();

  c.symbol = a.value("symbol").toString();
  c.packages = a.value("packages").toString().split(" ");
  
  sr.skipCurrentElement();
  return sr;
}

QXmlStreamWriter &operator<<(QXmlStreamWriter &sw, PkgRule const &c) {
  sw.writeStartElement("rule");
  sw.writeAttribute("symbol", c.symbol);
  sw.writeAttribute("packages", packages.join(" "));
  sw.writeEndElement();
  return sw;
};
  
QString PkgRule::report() const {
  return QString("%1: %2").arg(symbol).arg(packages.join(", "));
}
