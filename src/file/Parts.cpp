// Parts.cpp

#include "Parts.h"
#include <QDebug>

class PartsData: public QSharedData {
public:
  PartsData() { }
public:
  QMap<int, Container> containers;
  QMap<int, Package> packages;
};


Parts::Parts() {
  d = new PartsData;
}

Parts::Parts(Parts const &o) {
  d = o.d;
}

Parts::Parts(QXmlStreamReader &src): Parts() {
  while (!src.atEnd()) {
    src.readNext();
    if (src.isStartElement()) {
      if (src.name()=="container") {
        Container c(src);
        d->containers[c.id()] = c;
      } else if (src.name()=="package") {
        Package c(src);
        d->packages[c.id()] = c;
      } else {
        qDebug() << "Unexpected element in parts: " << src.name();
      }
    } else if (src.isEndElement()) {
      break;
    } else if (src.isCharacters() && src.isWhitespace()) {
    } else if (src.isComment()) {
    } else {
      qDebug() << "Unexpected entity in parts: " << src.tokenType();
    }
  }
  // now at end of parts element
}

Parts &Parts::operator=(Parts const &o) {
  d = o.d;
  return *this;
}

Parts::~Parts() {
}

QMap<int, class Container> const &Parts::containers() const {
  return d->containers;
}

QMap<int, class Container> &Parts::containers() {
  d.detach();
  return d->containers;
}

QMap<int, class Package> const &Parts::packages() const {
  return d->packages;
}

QMap<int, class Package> &Parts::packages() {
  d.detach();
  return d->packages;
}

QXmlStreamReader &operator>>(QXmlStreamReader &sr, Parts &c) {
  c = Parts(sr);
  return sr;
}
  
QXmlStreamWriter &operator<<(QXmlStreamWriter &sr, Parts const &c) {
  sr.writeStartElement("parts");
  for (auto const &c: c.containers())
    sr << c;
  for (auto const &c: c.packages())
    sr << c;
  sr.writeEndElement();
  return sr;
}

bool Parts::isEmpty() const {
  return containers().isEmpty() && packages().isEmpty();
}
