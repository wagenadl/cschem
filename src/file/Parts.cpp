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

QMap<int, class Package> const &Parts::packages() const {
  return d->packages;
}

void Parts::insert(Package const &pkg) {
  d.detach();
  d->packages[pkg.id()] = pkg;
}

void Parts::insert(Container const &cnt) {
  d.detach();
  d->containers[cnt.id()] = cnt;
}

void Parts::removePackage(int id) {
  d.detach();
  d->packages.remove(id);
}

void Parts::removeContainer(int id) {
  d.detach();
  d->containers.remove(id);
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

void Parts::renumber(QMap<int, int> map) {
  d.detach();
  
  QList<Container> cons;
  for (auto const &c: containers())
    cons << c;
  
  QList<Package> pkgs;
  for (auto const &p: packages())
    pkgs << p;

  for (auto &c: cons) 
    if (map.contains(c.id()))
      c.setId(map[c.id()]);

  for (auto &p: pkgs) 
    if (map.contains(p.id()))
      p.setId(map[p.id()]);

  d->containers.clear();
  d->packages.clear();

  for (auto &c: cons)
    d->containers[c.id()] = c;
  for (auto &p: pkgs)
    d->packages[p.id()] = p;
}
