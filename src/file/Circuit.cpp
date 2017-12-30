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

void Circuit::insert(Element const &e) {
  d.detach();
  d->elements[e.id()] = e;
}

void Circuit::insert(Connection const &e) {
  d.detach();
  d->connections[e.id()] = e;
}

void Circuit::remove(int id) {
  d.detach();
  if (d->elements.contains(id)) {
    d->elements.remove(id);
    QList<int> cids;
    for (auto const &c: connections()) 
      if (c.fromId()==id || c.toId()==id)
        cids << c.id();
    for (int cid: cids)
      d->connections.remove(cid);
  } else if (d->connections.contains(id)) {
    d->connections.remove(id);
  } else {
    qDebug() << "Nothing to remove for " << id;
  }
}

QSet<int> Circuit::connectionsOn(int id, QString pin) const {
  QSet<int> cids;
  for (auto const &c: d->connections) 
    if ((c.fromId()==id && c.fromPin()==pin)
        || (c.toId()==id && c.toPin()==pin))
      cids << c.id();
  return cids;
}

QSet<int> Circuit::connectionsTo(QSet<int> ids) const {
  QSet<int> cids;
  for (auto const &c: d->connections) 
    if (ids.contains(c.toId()))
      cids << c.id();
  return cids;
}

QSet<int> Circuit::connectionsFrom(QSet<int> ids) const {
  QSet<int> cids;
  for (auto const &c: d->connections) 
    if (ids.contains(c.fromId()))
      cids << c.id();
  return cids;
}

QSet<int> Circuit::connectionsIn(QSet<int> ids) const {
  QSet<int> cids;
  for (auto const &c: d->connections) 
    if (ids.contains(c.toId()) && ids.contains(c.fromId()))
      cids << c.id();
  return cids;
}

void Circuit::translate(QSet<int> ids, QPoint delta) {
  d.detach();
  for (int id: ids)
    d->elements[id].setPosition(d->elements[id].position() + delta);
  for (int id: connectionsIn(ids)) {
    QList<QPoint> &via(d->connections[id].via());
    for (QPoint &p: via)
      p += delta;
  }
}

Element const &Circuit::element(int id) const {
  static Element ne;
  auto it(d->elements.find(id));
  return it == d->elements.end() ? ne : *it;
}

Element &Circuit::element(int id) {
  d.detach();
  return d->elements[id];
}

Connection const &Circuit::connection(int id) const {
  static Connection ne;
  auto it(d->connections.find(id));
  return it == d->connections.end() ? ne : *it;
}

Connection &Circuit::connection(int id) {
  d.detach();
  return d->connections[id];
}
