// Circuit.cpp

#include "Circuit.h"
#include <QDebug>

class CircuitData: public QSharedData {
public:
  CircuitData() { }
public:
  QMap<int, Component> components;
  QMap<int, Port> ports;
  QMap<int, Junction> junctions;
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
      if (src.name()=="component") {
        Component c(src);
        d->components[c.id()] = c;
      } else if (src.name()=="port") {
        Port c(src);
        d->ports[c.id()] = c;
      } else if (src.name()=="junction") {
        Junction c(src);
        d->junctions[c.id()] = c;
      } else if (src.name()=="connection") {
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

QMap<int, class Component> const &Circuit::components() const {
  return d->components;
}

QMap<int, class Component> &Circuit::components() {
  return d->components;
}

QMap<int, class Port> const &Circuit::ports() const {
  return d->ports;
}

QMap<int, class Port> &Circuit::ports() {
  return d->ports;
}

QMap<int, class Junction> const &Circuit::junctions() const {
  return d->junctions;
}
  
QMap<int, class Junction> &Circuit::junctions() {
  return d->junctions;
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
  for (auto const &c: c.components())
    sr << c;
  for (auto const &c: c.ports())
    sr << c;
  for (auto const &c: c.junctions())
    sr << c;
  for (auto const &c: c.connections())
    sr << c;
  sr.writeEndElement();
  return sr;
}
