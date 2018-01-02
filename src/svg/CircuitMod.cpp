// CircuitMod.cpp

#include "CircuitMod.h"
#include "Geometry.h"
#include "file/Circuit.h"
#include "file/PinID.h"
#include "svg/Router.h"
#include <QDebug>

struct OverlapResult {
  OverlapResult():
    overlap(false), allOfFirstSegmentA(false), allOfFirstSegmentB(false) { }
  operator bool() const { return overlap; }
  bool overlap;
  bool allOfFirstSegmentA;
  bool allOfFirstSegmentB;
};

class CircuitModData {
public:
  CircuitModData(Circuit const &circ, PartLibrary const *lib):
    circ(circ), lib(lib) {
  }
  OverlapResult overlappingStart(Connection const &a, Connection const &b) const;
  bool removePointlessJunction(int id);
  void removeOverlap(int ida, int idb, OverlapResult over);
  bool removeOverlappingJunctions(int id);
public:
  Circuit circ;
  PartLibrary const *lib;
  QSet<int> acons;
  QSet<int> aelts;
};


CircuitMod::CircuitMod(Circuit const &circ, PartLibrary const *lib):
  d(new CircuitModData(circ, lib)) {
}

CircuitMod::~CircuitMod() {
  delete d;
}

QSet<int> CircuitMod::affectedConnections() const {
  return d->acons;
}

QSet<int> CircuitMod::affectedElements() const {
  return d->aelts;
}

Circuit const &CircuitMod::circuit() const {
  return d->circ;
}

bool CircuitMod::deleteElement(int id) {
  if (!d->circ.elements().contains(id))
    return false;
  
  Geometry geom(d->circ, d->lib);
  for (auto c: d->circ.connections()) {
    if (c.fromId()==id) {
      c.setFromId(0); // make dangling
      c.via().prepend(geom.pinPosition(id, c.fromPin()));
      d->circ.insert(c);
      d->acons << c.id();
    }
    if (c.toId()==id) {
      c.setToId(0); // make dangling
      c.via().append(geom.pinPosition(id, c.toPin()));
      d->circ.insert(c);
      d->acons << c.id();
    }
  }

  d->aelts << id;
  d->circ.remove(id);
  return true;
}

bool CircuitModData::removePointlessJunction(int id) {
  if (circ.element(id).type() != Element::Type::Junction)
    return false;
  
  QList<int> cc = circ.connectionsOn(id, "").toList();

  qDebug() << "removepointless" << id;
  for (int c: cc)
    qDebug() << "  " << circ.connection(c).report();
  
  if (cc.size() > 2)
    return false;
  
  if (cc.size() == 2) {
    // reconnect what would be dangling
    Connection con0(circ.connection(cc[0]));
    Connection con1(circ.connection(cc[1]));

    if (con0.fromId() == id)
      con0.reverse();
    if (con1.toId() == id)
      con1.reverse();

    con0.setToId(con1.toId());
    con0.setToPin(con1.toPin());
    con0.via() << circ.element(id).position();
    con0.via() += con1.via();
    Geometry geom(circ, lib);
    QPolygon path = geom.simplifiedPath(geom.connectionPath(con0));
    path.removeFirst(); path.removeLast();
    con0.setVia(path);
    circ.insert(con0);
    circ.remove(cc[1]);
  }

  for (int c: cc)
    acons << c;
  aelts << id;

  circ.remove(id);

  return true;
}

bool CircuitMod::removePointlessJunction(int id) {
  return d->removePointlessJunction(id);
}

bool CircuitMod::deleteConnection(int id) {
  if (!d->circ.connections().contains(id))
    return false;

  Connection con(d->circ.connection(id));
  
  d->acons << id;
  d->circ.remove(id);
  
  int from = con.fromId();
  if (d->circ.element(from).type() == Element::Type::Junction)
    removePointlessJunction(from);

  int to = con.toId();
  if (d->circ.element(to).type() == Element::Type::Junction) 
    removePointlessJunction(to);

  return true;
}

bool CircuitMod::deleteConnectionSegment(int id, int seg) {
  if (!d->circ.connections().contains(id))
    return false;
  if (seg<0)
    return false;
  Connection con(d->circ.connection(id));
  QPolygon via = con.via();
  qDebug() << "deleteConnectionSegment" << id << seg << con.report();
  if (con.danglingStart())
    seg ++;
  // so seg=0 _always_ means before the vias, and seg=#via _always_ means
  // after the vias
  if (via.isEmpty()) {
    return deleteConnection(id);
  } else if (seg==0) { // first segment, before vias
    int from = con.fromId();
    con.setFrom(0); // make dangling
    if (con.isNull())
      d->circ.remove(id);
    else
      d->circ.insert(con);
    d->acons << id;
    removePointlessJunction(from);
    return true;
  } else if (seg>=via.size()) { // last segment, after vias
    int to = con.toId();
    con.setTo(0); // make dangling
    if (con.isNull())
      d->circ.remove(id);
    else
      d->circ.insert(con);
    d->acons << id;
    removePointlessJunction(to);
    return true;
  } else {
    // removing middle segment => split into two dangling parts
    Connection con1;
    con1.setTo(con.to());
    con.setTo(0);
    QPolygon via1;
    while (via.size() > seg)
      via1.prepend(via.takeLast());
    con.setVia(via);
    con1.setVia(via1);

    if (con.isNull())
      d->circ.remove(id);
    else
      d->circ.insert(con);
    d->acons << id;

    if (!con1.isNull()) {
      d->circ.insert(con1);
      d->acons << con1.id();
    }

    return true;
  }
}

bool CircuitMod::removeConnectionsEquivalentTo(int id) {
  Connection con(d->circ.connection(id));
  PinID from = con.from();
  PinID to = con.to();
  QSet<int> cc;
  for (auto const &c: d->circ.connections())
    if (c.id()!=id
        && ((c.from()==from && c.to()==to)
            || (c.from()==to && c.to()==from)))
      cc << c.id();

  bool res = false;
  for (int c: cc)
    if (deleteConnection(c))
      res = true;
  
  return res;
}

bool CircuitMod::removeIfDangling(int id) {
  if (d->circ.connection(id).isDangling()) 
    return deleteConnection(id);
  else
    return false;
}

bool CircuitMod::removeAllDanglingOrCircular() {
  bool any = false;
  while (true) {
    /* This odd while loop is needed, because deleting a connection
       can effectively change IDs of other connections through
       deletion of a junction. */
    bool now = false;
    for (auto const &c: d->circ.connections())
      if (c.isDangling() || c.isCircular()) {
        now = true;
        deleteConnection(c.id());
        break;
      }
    if (now)
      any = true;
    else
      break;
  }
  return any;
}

bool CircuitMod::removeIfCircular(int id) {
  if (d->circ.connection(id).isCircular()) 
    return deleteConnection(id);
  else
    return false;
}

bool CircuitMod::simplifyConnection(int id) {
  if (!d->circ.connections().contains(id))
    return false;
  Geometry geom(d->circ, d->lib);
  QPolygon path0(geom.connectionPath(id));
  QPolygon path1 = Geometry::simplifiedPath(path0);
  if (path1.size() == path0.size())
    return false;
  Connection con(d->circ.connection(id));
  if (con.fromId()>0)
    path1.removeFirst();
  if (con.toId()>0)
    path1.removeLast();
  con.setVia(path1);
  d->circ.insert(con);
  return true;
}

bool CircuitModData::removeOverlappingJunctions(int id) {
  Element junc(circ.element(id));
  if (junc.type() != Element::Type::Junction)
    return false;
  
  QSet<int> jj;
  for (int c: circ.connectionsOn(id, "")) {
    Connection con(circ.connection(c));
    int id1 = con.fromId()==id ? con.toId() : con.fromId();
    Element junc1(circ.element(id1));
    if (junc1.type() == Element::Type::Junction
	&& id1 != id
	&& junc1.position() == junc.position())
      jj << id1;
  }
  if (jj.isEmpty())
    return false;
  
  for (int j: jj) {
    for (int c: circ.connectionsOn(j, "")) {
      Connection con(circ.connection(c));
      if (con.fromId()==j) {
	con.setFromId(id);
	if (con.isCircular())
	  circ.remove(c);
	else
	  circ.insert(con);
	acons << c;
      } 
      if (con.toId()==j) {
	con.setToId(id);
	if (con.isCircular())
	  circ.remove(c);
	else
	  circ.insert(con);
	acons << c;
      }
    }
    circ.remove(j);
    aelts << j;
  }
  return true;
}

void CircuitModData::removeOverlap(int ida, int idb, OverlapResult over) {
  if (!over)
    return;
  Connection a = circ.connection(ida);
  Connection b = circ.connection(idb);
  Geometry geom(circ, lib);
  QPolygon patha = geom.connectionPath(a);
  QPolygon pathb = geom.connectionPath(b);
  patha.removeFirst();
  pathb.removeFirst();
  QPoint joint;
  if (over.allOfFirstSegmentA) 
    joint = patha.takeFirst();
  if (over.allOfFirstSegmentB) 
    joint = pathb.takeFirst();
  /* Note that at least one of the above two conditions _must_ be true
     by construction.  Further, note that it is possible that we
     gobbled up the "to" pin's position if the shorter segment is the
     only segment of its connection. This is a special case. Normally,
     we create a new junction at the end of the shorter segment, and deal
     with the fallout. But now we risk placing to junctions on top of
     each other.
  */

  if (a.toId()>0 && !patha.isEmpty())
    patha.takeLast();
  if (b.toId()>0 && !pathb.isEmpty())
    pathb.takeLast();

  // Create new junction at joint and new connection from start point to there.
  Element j = Element::junction(joint);
  Connection c;
  c.setFrom(a.from());
  c.setTo(j.id(), "");
  circ.insert(j);
  circ.insert(c);
  acons << c.id();
  aelts << j.id();

  // Reroute both original connections to start at joint.
  a.setFrom(j.id(), "");
  b.setFrom(j.id(), "");
  a.setVia(patha);
  b.setVia(pathb);
  circ.insert(a);
  if (b.isCircular())
    circ.remove(b.id());
  else
    circ.insert(b);
  acons << a.id();
  acons << b.id();

  qDebug() << "overlapremoved";
  qDebug() << "  " << a.report();
  qDebug() << "  " << b.report();

  // The original starting point may have become a useless junction, so:
  removePointlessJunction(c.fromId());

  removeOverlappingJunctions(j.id());
  if (removePointlessJunction(j.id()))
    aelts.remove(j.id());
}

OverlapResult CircuitModData::overlappingStart(Connection const &a,
                                               Connection const &b) const {
  OverlapResult res;
  if (a.id()==b.id() || a.fromId()<=0 || a.from()!=b.from())
    return res;
  Geometry geom(circ, lib);
  qDebug() << "overlappingstart?" << a.report() << b.report();
  QPolygon patha(geom.connectionPath(a));
  QPolygon pathb(geom.connectionPath(b));
  // By construction, the first points in the path are the same
  QPoint deltaa = patha[1] - patha[0];
  QPoint deltab = pathb[1] - pathb[0];
  qDebug() << " patha" << patha << "length" << deltaa;
  qDebug() << " pathb" << pathb << "length" << deltab;
  res.overlap
    = (deltaa.x()==0 && deltab.x()==0 && deltaa.y()*deltab.y()>0)
    || (deltaa.y()==0 && deltab.y()==0 && deltaa.x()*deltab.x()>0);
  if (!res.overlap)
    return res;

  int la = deltaa.manhattanLength();
  int lb = deltab.manhattanLength();

  qDebug() << " => " << la << lb;
  if (la<=lb)
    res.allOfFirstSegmentA = true;
  if (lb<=la)
    res.allOfFirstSegmentB = true;
  return res;
}


bool CircuitMod::adjustOverlappingConnections(int id) {
  bool res = removeConnectionsEquivalentTo(id);
  if (simplifyConnection(id))
    res = true;

  bool keepgoing = true;
  while (keepgoing) {
    keepgoing = false;
    Connection a(d->circ.connection(id));
    
    for (auto const &b: d->circ.connections()) {
      if (b.id()==id)
	continue;
      OverlapResult over;
      over = d->overlappingStart(a, b);
      if (over) {
	d->removeOverlap(a.id(), b.id(), over);
	keepgoing = true;
	res = true;
	break;
      }
      Connection br = b.reversed();
      over = d->overlappingStart(a, br);
      if (over) {
	d->acons << b.id();
	d->circ.insert(br);
	d->removeOverlap(a.id(), b.id(), over);
	keepgoing = true;
	res = true;
	break;
      }
      Connection ar = a.reversed();
      over = d->overlappingStart(ar, b);
      if (over) {
	d->acons << a.id();
	d->circ.insert(ar);
	d->removeOverlap(a.id(), b.id(), over);
	keepgoing = true;
	res = true;
	break;
      }
      over = d->overlappingStart(ar, br);
      if (over) {
	d->acons << a.id();
	d->acons << b.id();
	d->circ.insert(ar);
	d->circ.insert(br);
	d->removeOverlap(a.id(), b.id(), over);
	keepgoing = true;
	res = true;
	break;
      }
    }
  }
  return res;
}

bool CircuitMod::translateConnection(int id, QPoint dd) {
  if (!d->circ.connections().contains(id))
    return false;
  d->circ.insert(d->circ.connection(id).translated(dd));
  d->acons << id;
  return true;
}

bool CircuitMod::translateElement(int id, QPoint dd) {
  if (!d->circ.elements().contains(id))
    return false;
  d->circ.insert(d->circ.element(id).translated(dd));
  d->aelts << id;
  return true;
}

bool CircuitMod::reroute(int id, Circuit const &origcirc) {
  if (!d->circ.connections().contains(id))
    return false;
  Router router(d->lib);
  d->circ.insert(router.reroute(id, origcirc, d->circ));
  d->acons << id;
  return true;
}

bool CircuitMod::removeOverlappingJunctions(int eltid) {
  return d->removeOverlappingJunctions(eltid);
}