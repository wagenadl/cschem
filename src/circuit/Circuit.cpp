// Circuit.cpp

#include "Circuit.h"
#include <QDebug>
#include "PinID.h"
#include "PartNumbering.h"
#include <QRegularExpression>

Circuit::Circuit() {
  valid = true;
}

Circuit::Circuit(Connection const &con): Circuit() {
  insert(con);
}

int Circuit::elementByName(QString name) const {
  for (auto it=elements.begin(); it!=elements.end(); ++it)
    if (it.value().name==name)
      return it.key();
  return -1;
}

QXmlStreamReader &operator>>(QXmlStreamReader &sr, Circuit &c) {
  while (!sr.atEnd()) {
    sr.readNext();
    if (sr.isStartElement()) {
      auto n = sr.name();
      if (n=="component" || n=="port" || n=="junction") {
	Element elt;
	sr >> elt;
	c.elements.insert(elt.id, elt);
      } else if (n=="connection") {
	Connection con;
	sr >> con;
	c.connections.insert(con.id, con);
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
  if (!e.isValid()) {
    qDebug() << "Inserting invalid Element";
  }
  elements.insert(e.id, e);
}

void Circuit::insert(Connection const &c) {
  if (!c.isValid()) {
    qDebug() << "Inserting invalid connection";
  }
  connections.insert(c.id, c);
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
    elements.insert(start, elt);
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
      connections.insert(start, con);
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
    elements.insert(elt.id, elt);
  for (auto con: o.connections)
    connections.insert(con.id, con);
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

QDebug &operator<<(QDebug &dbg, Circuit const &circ) {
  dbg << "Elements:\n";
  for (auto const &e: circ.elements)
    dbg << "  " << e << "\n";
  dbg << "Connections:\n";
  for (auto const &c: circ.connections)
    dbg << "  " << c << "\n";
  return dbg;
}

QStringList Circuit::conflictingNames() const {
  QMap<QString, int> name2container;
  QMap<QString, int> name2contents;
  QSet<QString> conflicted;
  for (Element const &elt: elements) {
    int id = elt.id;
    QString name = elt.name;
    bool isc = elt.isContainer();
    if (isc) {
      if (name2container.contains(name))
	conflicted << name;
      name2container[name] = id;
    } else {
      if (name2contents.contains(name))
	conflicted << name;
      name2contents[name] = id;
    }
  }
  return conflicted.toList();
}


QSet<int> Circuit::containedElements(int id) const {
  QSet<int> ids;
  if (!elements.contains(id))
    return ids;
  Element const &elt = elements[id];
  if (!elt.isContainer())
    return ids;
  QString name = elt.name;
  for (Element const &e: elements) {
    if (e.id==id)
      continue;
    if (e.name==name) {
      ids << e.id;
    } else {
      int idx = e.name.indexOf(".");
      if (idx>0 && e.name.left(idx)==name)
	ids << e.id;
    }
  }
  return ids;
}

int Circuit::containerOf(int id) const {
  if (!elements.contains(id))
    return -1;
  Element const &elt = elements[id];
  QString name = elt.name;
  int idx = name.indexOf(".");
  QString cname = idx>0 ? name.left(idx) : name;
  for (Element const &e: elements) {
    if (e.id!=id)
      continue;
    if (e.name == cname && e.isContainer())
      return e.id;
  }
  return -1;
}  

bool Circuit::resolveConflictingNames(QList<int> &affected_ids_out) {
  QMap<QString, QList<int>> name2container;
  QMap<QString, QList<int>> name2contents;
  for (Element const &elt: elements) {
    int id = elt.id;
    QString name = elt.name;
    bool isc = elt.isContainer();
    if (isc) 
      name2container[name] << id;
    else
      name2contents[name] << id;
  }
  
  for (QString name: name2container.keys()) {
    if (name2container[name].size()>=2) {
      for (int id: name2container[name])
	if (!containedElements(id).isEmpty())
	  return false; // cannot automatically renumber nonempty containers
    } else {
      name2container.remove(name);
    }
  }

  for (QString name: name2contents.keys()) {
    if (name2contents[name].size()>=2) {
      for (int id: name2contents[name])
	if (containerOf(id)>0)
	  return false; // cannot automatically renumber contained elements
    } else {
      name2contents.remove(name);
    }
  }

  affected_ids_out.clear();

  if (name2container.isEmpty() && name2contents.isEmpty())
    return true; // easy

  QRegularExpression re_dig("\\d");  

  for (QString name: name2container.keys())
    if (name.indexOf(re_dig)<0)
      return false; // must have a number to renumber
  for (QString name: name2contents.keys())
    if (name.indexOf(re_dig)<0)
      return false; // must have a number to renumber

  QMap<QString, QSet<int>> usedNumbers;
  for (Element const &e: elements) {
    QString name = e.name;
    int numidx = name.indexOf(re_dig);
    if (numidx<0)
      continue;
    QString pfx = name.left(numidx);
    QString numbit = name.mid(numidx);
    int dotidx = numbit.indexOf(".");
    if (dotidx<0)
      usedNumbers[pfx] << numbit.toInt();
    else
      usedNumbers[pfx] << numbit.left(dotidx).toInt();
  }
  
  auto firstFree = [&](QString pfx) {
    QSet<int> const &used(usedNumbers[pfx]);
    int first = 1;
    while (used.contains(first))
      first ++;
    usedNumbers[pfx] << first;
    return first;
  };

  for (QString name: name2container.keys()) {
    QList<int> ids = name2container[name];
    ids.removeFirst();
    int numidx = name.indexOf(re_dig);
    QString pfx = name.left(numidx);
    for (int id: ids) {
      int newnum = firstFree(pfx);
      elements[id].name = pfx + QString::number(newnum);
      affected_ids_out << id;
      QSet<int> ids = containedElements(id);
      for (int id1: ids) {
	int idx = elements[id1].name.indexOf(".");
	if (idx>0)
	  elements[id1].name = pfx + QString::number(newnum)
	    + elements[id1].name.mid(idx);
	else
	  elements[id1].name = pfx + QString::number(newnum);
	affected_ids_out << id1;
      }
    }
  }

  for (QString name: name2contents.keys()) {
    QList<int> ids = name2contents[name];
    ids.removeFirst();
    int numidx = name.indexOf(re_dig);
    QString pfx = name.left(numidx);
    int dotidx = name.indexOf(".", numidx);
    QString sfx;
    if (dotidx>0)
      sfx = name.mid(dotidx);
    for (int id: ids) {
      int newnum = firstFree(pfx);
      elements[id].name = pfx + QString::number(newnum) + sfx;
      affected_ids_out << id;
      int id1 = containerOf(id);
      if (id1>0) {
	elements[id1].name = pfx + QString::number(newnum);
	affected_ids_out << id1;
      }
    }
  }

  return true;
}
