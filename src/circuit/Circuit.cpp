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
  valid = true;
}

Circuit::Circuit(Connection const &con): Circuit() {
  insert(con);
}
  
QXmlStreamReader &operator>>(QXmlStreamReader &sr, Circuit &c) {
  while (!sr.atEnd()) {
    sr.readNext();
    if (sr.isStartElement()) {
      auto n = sr.name();
      if (n=="component" || n=="port" || n=="junction") {
	Element elt;
	sr >> elt;
	c.elements[elt.id] = elt;
      } else if (n=="connection") {
	Connection con;
	sr >> con;
	c.connections[con.id] = con;
      } else {
	qDebug() << "Unexpected element in circuit: " << sr.name();
	c.invalidate();
      }
    } else if (sr.isEndElement()) {
      break;
    } else if (sr.isCharacters() && sr.isWhitespace()) {
    } else if (sr.isComment()) {
    } else {
      qDebug() << "Unexpected entity in circuit: " << sr.tokenType();
      c.invalidate();
    }
  }
  // now at end of circuit element
  return sr;
}
  
QXmlStreamWriter &operator<<(QXmlStreamWriter &sr, Circuit const &c) {
  sr.writeStartElement("circuit");
  for (auto const &c: c.elements)
    sr << c;
  for (auto const &c: c.connections)
    sr << c;
  sr.writeEndElement();
  return sr;
}

void Circuit::insert(Element const &e) {
  elements[e.id] = e;
}

void Circuit::insert(Connection const &c) {
  if (!c.isValid()) {
    qDebug() << "Inserting invalid connection";
  }
  connections[c.id] = c;
}

void Circuit::removeElementWithConnections(int id) {
  if (elements.contains(id)) {
    elements.remove(id);
    QList<int> cids;
    for (auto const &c: connections) 
      if (c.fromId==id || c.toId==id)
        cids << c.id;
    for (int cid: cids)
      connections.remove(cid);
  } else {
    qDebug() << "Nothing to remove for " << id;
  }
}

QSet<int> Circuit::connectionsOn(PinID const &pid) const {
  return connectionsOn(pid.element(), pid.pin());
}

QSet<int> Circuit::connectionsOn(int id, QString pin) const {
  QSet<int> cids;
  for (auto const &c: connections) 
    if ((c.fromId==id && c.fromPin==pin)
        || (c.toId==id && c.toPin==pin))
      cids << c.id;
  return cids;
}

QSet<int> Circuit::connectionsOn(int id) const {
  QSet<int> cids;
  for (auto const &c: connections) 
    if (c.fromId==id || c.toId==id)
      cids << c.id;
  return cids;
}

QSet<int> Circuit::connectionsTo(QSet<int> ids) const {
  QSet<int> cids;
  for (auto const &c: connections) 
    if (ids.contains(c.toId))
      cids << c.id;
  return cids;
}

QSet<int> Circuit::connectionsFrom(QSet<int> ids) const {
  QSet<int> cids;
  for (auto const &c: connections) 
    if (ids.contains(c.fromId))
      cids << c.id;
  return cids;
}

QSet<int> Circuit::connectionsIn(QSet<int> ids) const {
  QSet<int> cids;
  for (auto const &c: connections) {
    int from = c.fromId;
    int to = c.toId;
    bool gotfrom = ids.contains(from);
    bool gotto = ids.contains(to);
    if ((gotfrom && gotto)
	|| (gotfrom && to<=0)
	|| (gotto && from<=0))
      cids << c.id;
  }
  return cids;
}

void Circuit::translate(QSet<int> ids, QPoint delta) {
  for (int id: ids)
    if (elements.contains(id))
      elements[id].position += delta;
  for (int id: connectionsIn(ids))
    for (QPoint &p: connections[id].via)
      p += delta;
}

void Circuit::translate(QPoint delta) {
  for (Element &elt: elements)
    elt.position += delta;
  for (Connection &con: connections)
    con.via.translate(delta);
}

int Circuit::renumber(int start, QMap<int, int> *mapout) {
  QMap<int, int> eltmap;

  QList<Element> elts = elements.values();
  elements.clear();

  for (Element elt: elts) {
    int oldid = elt.id;
    elt.id = start;
    elements.remove(oldid);
    elements[start] = elt;
    eltmap[oldid] = start;
    start ++;
  }

  QList<Connection> cons = connections.values();
  connections.clear();
  
  for (Connection con: cons) {
    con.id = start;
    if (eltmap.contains(con.fromId))
      con.fromId = eltmap[con.fromId];
    else if (con.fromId>0) {
      con.setFrom(0); // make dangling
      qDebug() << "Circuit::renumber: Disconnecting from nonexistent element";
    }
    if (eltmap.contains(con.toId))
      con.toId = eltmap[con.toId];
    else if (con.toId>0) {
      con.setTo(0); // make dangling
      qDebug() << "Circuit::renumber: Disconnecting from nonexistent element";
    }
    if (con.isValid()) {
      connections[start] = con;
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
    if (elements.contains(e))
      circ.insert(elements[e]);
  QSet<int> intcon = connectionsIn(elts);
  QSet<int> extcon = (connectionsFrom(elts) + connectionsTo(elts)) - intcon;
  for (int c: intcon)
    circ.insert(connections[c]);
  for (int c: extcon)
    if (connections[c].isDangling())
      circ.insert(connections[c]);
  return circ;
}

bool Circuit::isEmpty() const {
  return elements.isEmpty();
}

bool Circuit::isValid() const {
  return valid;
}

void Circuit::invalidate() {
  valid = false;
}

int Circuit::maxId() const {
  int mx = 0;
  for (int id: elements.keys())
    if (id>mx)
      mx = id;
  for (int id: connections.keys())
    if (id>mx)
      mx = id;
  return mx;
}

void Circuit::merge(Circuit const &o) {
  for (auto elt: o.elements)
    elements[elt.id] = elt;
  for (auto con: o.connections)
    connections[con.id] = con;
}
  
int Circuit::availableNumber(QString pfx) const {
  QSet<int> used;
  for (Element const &elt: elements) 
    if (elt.name.startsWith(pfx)) 
      used << elt.name.mid(pfx.size()).toInt();

  int n = 1;
  while (used.contains(n))
    n++;

  return n;
}

QString Circuit::autoName(QString sym) const {
  if (sym.startsWith("port:")) {
    return "";
  } else if (sym.startsWith("part:")) {
    QString pfx = PartNumbering::abbreviation(sym);
    return pfx + QString::number(availableNumber(pfx));
  } else {
    return "";
  }
}
