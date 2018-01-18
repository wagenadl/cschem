// Circuit.cpp

#include "Circuit.h"
#include <QDebug>
#include "PinID.h"
#include "PartNumbering.h"

class CircuitData: public QSharedData {
public:
  CircuitData(bool valid=true): valid(valid) { }
public:
  QMap<int, Element> elements;
  QMap<int, Connection> connections;
  bool valid;
};  

Circuit::Circuit() {
  d = new CircuitData;
}

Circuit::Circuit(Circuit const &o) {
  d = o.d;
}

Circuit::Circuit(Connection const &con): Circuit() {
  insert(con);
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

QMap<int, class Connection> const &Circuit::connections() const {
  return d->connections;
}
  
QXmlStreamReader &operator>>(QXmlStreamReader &sr, Circuit &c) {
  while (!sr.atEnd()) {
    sr.readNext();
    if (sr.isStartElement()) {
      auto n = sr.name();
      if (n=="component" || n=="port" || n=="junction") {
	Element elt;
	sr >> elt;
	c.d->elements[elt.id()] = elt;
      } else if (n=="connection") {
	Connection con;
	sr >> con;
	c.d->connections[con.id()] = con;
      } else {
	qDebug() << "Unexpected element in circuit: " << sr.name();
	c.d->valid = false;
      }
    } else if (sr.isEndElement()) {
      break;
    } else if (sr.isCharacters() && sr.isWhitespace()) {
    } else if (sr.isComment()) {
    } else {
      qDebug() << "Unexpected entity in circuit: " << sr.tokenType();
      c.d->valid = false;
    }
  }
  // now at end of circuit element
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

void Circuit::insert(Connection const &c) {
  d.detach();
  if (!c.isValid()) {
    qDebug() << "Inserting invalid connection";
  }
  d->connections[c.id()] = c;
}

void Circuit::removeElement(int id) {
  if (d->elements.contains(id)) {
    d.detach();
    d->elements.remove(id);
    QList<int> cids;
    for (auto const &c: connections()) 
      if (c.fromId()==id || c.toId()==id)
        cids << c.id();
    for (int cid: cids)
      d->connections.remove(cid);
  } else {
    qDebug() << "Nothing to remove for " << id;
  }
}

void Circuit::removeConnection(int id) {
  if (d->connections.contains(id)) {
    d.detach();
    d->connections.remove(id);
  } else {
    qDebug() << "Nothing to remove for " << id;
  }
}

QSet<int> Circuit::connectionsOn(PinID const &pid) const {
  return connectionsOn(pid.element(), pid.pin());
}

QSet<int> Circuit::connectionsOn(int id, QString pin) const {
  QSet<int> cids;
  for (auto const &c: d->connections) 
    if ((c.fromId()==id && c.fromPin()==pin)
        || (c.toId()==id && c.toPin()==pin))
      cids << c.id();
  return cids;
}


QSet<int> Circuit::connectionsOn(int id) const {
  QSet<int> cids;
  for (auto const &c: d->connections) 
    if (c.fromId()==id || c.toId()==id)
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
  for (auto const &c: d->connections) {
    int from = c.fromId();
    int to = c.toId();
    bool gotfrom = ids.contains(from);
    bool gotto = ids.contains(to);
    if ((gotfrom && gotto)
	|| (gotfrom && to<=0)
	|| (gotto && from<=0))
      cids << c.id();
  }
  return cids;
}

void Circuit::translate(QSet<int> ids, QPoint delta) {
  d.detach();
  for (int id: ids)
    if (d->elements.contains(id))
      d->elements[id].setPosition(d->elements[id].position() + delta);
  for (int id: connectionsIn(ids)) {
    QPolygon &via(d->connections[id].via());
    for (QPoint &p: via)
      p += delta;
  }
}

void Circuit::translate(QPoint delta) {
  d.detach();
  for (Element &elt: d->elements)
    elt.setPosition(elt.position() + delta);
  for (Connection &con: d->connections)
    con.setVia(con.via().translated(delta));
}

Element const &Circuit::element(int id) const {
  static Element ne;
  auto it(d->elements.find(id));
  return it == d->elements.end() ? ne : *it;
}

Connection const &Circuit::connection(int id) const {
  static Connection ne;
  auto it(d->connections.find(id));
  return it == d->connections.end() ? ne : *it;
}

int Circuit::renumber(int start, QMap<int, int> *mapout) {
  QMap<int, int> eltmap;

  QList<Element> elts = d->elements.values();
  d->elements.clear();

  for (Element elt: elts) {
    int oldid = elt.id();
    elt.setId(start);
    d->elements.remove(oldid);
    d->elements[start] = elt;
    eltmap[oldid] = start;
    start ++;
  }

  QList<Connection> cons = d->connections.values();
  d->connections.clear();
  
  for (Connection con: cons) {
    con.setId(start);
    int from = con.fromId();
    if (eltmap.contains(from))
      con.setFromId(eltmap[from]);
    else if (from>0) {
      con.setFromId(0); // make dangling
      qDebug() << "Circuit::renumber: Disconnecting from nonexistent element";
    }
    int to = con.toId();
    if (eltmap.contains(to))
      con.setToId(eltmap[to]);
    else if (to>0) {
      con.setToId(0); // make dangling
      qDebug() << "Circuit::renumber: Disconnecting from nonexistent element";
    }
    if (con.isValid()) {
      d->connections[start] = con;
      start++;
    }
  }

  if (mapout)
    *mapout = eltmap;
  
  return start - 1;
}

Circuit Circuit::subset(QSet<int> elts) const {
  Circuit circ;
  for (int e: elts)
    if (d->elements.contains(e))
      circ.insert(d->elements[e]);
  QSet<int> intcon = connectionsIn(elts);
  QSet<int> extcon = (connectionsFrom(elts) + connectionsTo(elts)) - intcon;
  for (int c: intcon)
    circ.insert(d->connections[c]);
  for (int c: extcon)
    if (d->connections[c].isDangling())
      circ.insert(d->connections[c]);
  return circ;
}

bool Circuit::isEmpty() const {
  return elements().isEmpty();
}

bool Circuit::isValid() const {
  return d->valid;
}

int Circuit::maxId() const {
  int mx = 0;
  for (int id: d->elements.keys())
    if (id>mx)
      mx = id;
  for (int id: d->connections.keys())
    if (id>mx)
      mx = id;
  return mx;
}

Circuit &Circuit::operator+=(Circuit const &o) {
  for (auto elt: o.elements())
    d->elements[elt.id()] = elt;
  for (auto con: o.connections())
    d->connections[con.id()] = con;
  return *this;
}
  
int Circuit::availableNumber(QString pfx) const {
  QSet<int> used;
  for (Element const &elt: elements()) 
    if (elt.name().startsWith(pfx)) 
      used << elt.name().mid(pfx.size()).toInt();

  int n = 1;
  while (used.contains(n))
    n++;

  return n;
}

QString Circuit::autoName(QString sym) const {
  if (sym.startsWith("port:")) {
    QStringList bits = sym.split(":");
    bits.removeFirst();
    if (bits.first() == "generic")
      return "V" + QString::number(availableNumber("V"));
    else
      return bits.last();
  } else if (sym.startsWith("part:")) {
    QString pfx = PartNumbering::abbreviation(sym);
    return pfx + QString::number(availableNumber(pfx));
  } else {
    return "nn";
  }
}
