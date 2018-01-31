// Packaging.cpp

#include "Packaging.h"

#include <QDebug>

QXmlStreamReader &operator>>(QXmlStreamReader &sr, Packaging &c) {
  c = Packaging();
  while (!sr.atEnd()) {
    sr.readNext();
    if (sr.isStartElement()) {
      auto n = sr.name();
      if (n=="rule") {
	PkgRule rule;
	sr >> rule;
	c.rules[rule.symbol] = rule;
      } else if (n=="package") {
	Package pkg;
	sr >> pkg;
	c.packages[pkg.name] = pkg;
      } else {
	qDebug() << "Unexpected element in packaging: " << sr.name();
      }
    } else if (sr.isEndElement()) {
      break;
    } else if (sr.isCharacters() && sr.isWhitespace()) {
    } else if (sr.isComment()) {
    } else {
      qDebug() << "Unexpected entity in packaging: " << sr.tokenType();
      c.d->valid = false;
    }
  }
  // now at end of circuit element
  return sr;
}
  
QXmlStreamWriter &operator<<(QXmlStreamWriter &sr, Circuit const &c) {
  sr.writeStartElement("packaging");
  for (auto const &c: c.rules)
    sr << c;
  for (auto const &c: c.packages)
    sr << c;
  sr.writeEndElement();
  return sr;
}
