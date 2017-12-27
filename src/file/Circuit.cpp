// Circuit.cpp

#include "Circuit.h"
#include <QDebug>

class CircuitData: public QSharedData {
public:
  CircuitData() { }
public:
  QMap<int, Element> elements;
  QMap<int, Connection> connections;
};  

Circuit::Circuit() {
  d = new CircuitData;
}

Circuit::Circuit(Circuit const &o) {
  d = o.d;
}

Circuit::Circuit(QXmlStreamReader &src): Circuit() {
  while (!src.atEnd()) {
    src.readNext();
    if (src.isStartElement()) {
      auto n = src.name();
      if (n=="component" || n=="port" || n=="junction") {
        Element c(src);
        d->elements[c.id()] = c;
      } else if (n=="connection") {
        Connection c(src);
        d->connections[c.id()] = c;
      } else {
        qDebug() << "Unexpected element in circuit: " << src.name();
      }
    } else if (src.isEndElement()) {
      break;
    } else if (src.isCharacters() && src.isWhitespace()) {
    } else if (src.isComment()) {
    } else {
      qDebug() << "Unexpected entity in circuit: " << src.tokenType();
    }
  }
  // now at end of circuit element
}

Circuit &Circuit::operator=(Circuit const &o) {
  d = o.d;
  return *this;
}

Circuit::~Circuit() {
}

QMap<int, class Element> const &Circuit::elements() const {
  return d->elements;
}

QMap<int, class Element> &Circuit::elements() {
  return d->elements;
}

QMap<int, class Connection> const &Circuit::connections() const {
  return d->connections;
}
  
QMap<int, class Connection> &Circuit::connections() {
  return d->connections;
}

QXmlStreamReader &operator>>(QXmlStreamReader &sr, Circuit &c) {
  c = Circuit(sr);
  return sr;
}
  
QXmlStreamWriter &operator<<(QXmlStreamWriter &sr, Circuit const &c) {
  sr.writeStartElement("circuit");
  for (auto const &c: c.elements())
    sr << c;
  for (auto const &c: c.connections())
    sr << c;
  sr.writeEndElement();
  return sr;
}
