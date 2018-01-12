// CircuitMod.cpp

#include "CircuitModData.h"

void CircuitModData::dropOrInsert(bool cond, Connection const &con) {
  insertOrDrop(!cond, con);
}

void CircuitModData::dropOrInsert(bool cond, Element const &elt) {
  insertOrDrop(!cond, elt);
}

void CircuitModData::insertOrDrop(bool cond, Connection const &con) {
  if (cond)
    insert(con);
  else
    drop(con);
}

void CircuitModData::insertOrDrop(bool cond, Element const &elt) {
  if (cond)
    insert(elt);
  else
    drop(elt);
}

void CircuitModData::insert(Connection const &con) {
  circ.insert(con);
  acons << con.id();
}

void CircuitModData::insert(Element const &elt) {
  circ.insert(elt);
  aelts << elt.id();
}

void CircuitModData::drop(Connection const &con) {
  circ.remove(con.id());
  acons << con.id();
}

void CircuitModData::drop(Element const &elt) {
  circ.remove(elt.id());
  aelts << elt.id();
}

void CircuitModData::dropCon(int con) {
  circ.remove(con);
  acons << con;
}

void CircuitModData::dropElt(int elt) {
  circ.remove(elt);
  aelts << elt;
}

bool CircuitModData::rewire(QSet<int> cc, PinID old, PinID new_) {
  bool any = false;
  for (int c: cc) {
    bool chg = false;
    Connection con(circ.connection(c));
    if (con.from()==old) {
      con.setFrom(new_);
      chg = true;
    }
    if (con.to()==old) {
      con.setTo(new_);
      chg = true;
    }
    if (chg) {
      insert(con);
      any = true;
    }
  }
  return any;
}

CircuitMod::CircuitMod(Circuit const &circ, PartLibrary const *lib):
  d(new CircuitModData(circ, lib)) {
}

CircuitMod::~CircuitMod() {
  delete d;
}

void CircuitMod::addElement(Element const &elt) {
  d->insert(elt);
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
  
  QSet<int> cc = d->circ.connectionsOn(id);

  bool isjunc = d->circ.element(id).type() == Element::Type::Junction;
  if (isjunc && cc.size()==2)
    return removePointlessJunction(id);
  if  (isjunc && cc.size()==4)
    // preserve passthroughs
    d->rewirePassthroughs(id, cc);
  else if (cc.size()>0) 
    d->makeDanglingAt(id, cc);

  d->dropElt(id);
  return true;
}

bool CircuitMod::deleteElements(QSet<int> elts) {
  if (elts.isEmpty())
    return false;
  
  QSet<int> intcon = d->circ.connectionsIn(elts);
  QSet<int> extcon
    = (d->circ.connectionsFrom(elts)
       + d->circ.connectionsTo(elts))
    - intcon;

  for (int id: extcon) 
    if (d->circ.connection(id).isDangling()) 
      d->dropCon(id);
  

  for (int id: intcon)
    d->dropCon(id);

  for (int id: elts)
    deleteElement(id);

  return true;
}
    
void CircuitModData::rewirePassthroughs(int id, QSet<int> cc) {
  if (cc.size() != 4) {
    qDebug() << "rewirepassthroughs requires precisely 4 connections";
    return;
  }
  QList<Connection> cons;
  for (int c: cc) {
    Connection con = circ.connection(c);
    if (con.fromId() != id)
      con.reverse();
    if (!con.isValid()) {
      qDebug() << "rewirepassthroughs requires valid connections";
      return;
    }
    qDebug() << "orig con" << con.report();
    cons << con;
  }
  // So now we have four connections that all start at the junction

  Geometry geom(circ, lib);
  
  QList<QPolygon> paths;
  for (int k=0; k<4; k++)
    paths << geom.connectionPath(cons[k]);

  QPolygon path0 = paths.takeFirst();
  Connection con0 = cons.takeFirst();
  QPoint dir0 = path0[1] - path0[0];
  int id1 = -1;
  for (int k=0; k<3; k++) {
    QPoint dirk = paths[k][1] - paths[k][0];
    if (QPoint::dotProduct(dir0, dirk) < 0) {
      // parallel connections
      con0.reverse(); // now *ends* at junction
      QPolygon path = geom.connectionPath(con0);
      path += paths[k];
      path = geom.simplifiedPath(path);
      con0.setTo(cons[k].to());
      con0.setVia(geom.viaFromPath(con0, path));
      id1 = cons[k].id();
      cons.removeAt(k);
      paths.removeAt(k);
      break;
    }
  }
  if (id1<=0) {
    qDebug() << "Could not find continuing connections";
    return;
  }
  insert(con0);
  dropCon(id1);

  // now join the other two
  con0 = cons.first();
  con0.reverse();
  QPolygon path = geom.connectionPath(con0);
  qDebug() << "path" << path;
  qDebug() << "pl" << paths.last();
  path += paths.last();
  path = geom.simplifiedPath(path);
  con0.setTo(cons.last().to());
  con0.setVia(geom.viaFromPath(con0, path));
  insert(con0);
  drop(cons.last());
}

void CircuitModData::makeDanglingAt(int id, QSet<int> cc) {
  Geometry geom(circ, lib);
  for (int k: cc) {
    Connection c = circ.connection(k);
    if (c.fromId()==id) {
      c.setFromId(0); // make dangling
      c.via().prepend(geom.pinPosition(id, c.fromPin()));
      insertOrDrop(c.isValid() && !geom.isZeroLength(c), c);
    }
    if (c.toId()==id) {
      c.setToId(0); // make dangling
      c.via().append(geom.pinPosition(id, c.toPin()));
      insertOrDrop(c.isValid() && !geom.isZeroLength(c), c);
    }
  }
}

bool CircuitModData::removePointlessJunction(int id) {
  if (circ.element(id).type() != Element::Type::Junction)
    return false;
  
  QList<int> cc = circ.connectionsOn(id, "").toList();
  
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
    insert(con0);
    dropCon(cc[1]);
  } else {
    for (int c: cc)
      dropCon(c);
  }
  dropElt(id);

  return true;
}

bool CircuitMod::removePointlessJunction(int id) {
  return d->removePointlessJunction(id);
}

bool CircuitMod::deleteConnection(int id) {
  if (!d->circ.connections().contains(id))
    return false;

  Connection con(d->circ.connection(id));

  d->drop(con);
  
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
  if (via.isEmpty()) 
    return deleteConnection(id);

  if (con.danglingStart())
    seg ++;
  // Now seg=0 _always_ means before the vias, and seg=#via _always_ means
  // after the vias
  Geometry geom(d->circ, d->lib);
  if (seg==0) { // First segment, before vias. This case is only possible
    // if our start is not dangling.
    int from = con.fromId();
    con.unsetFrom(); // make dangling
    d->dropOrInsert(geom.isZeroLength(con), con);
    removePointlessJunction(from);
    return true;
  } else if (seg>=via.size()) { // last segment, after vias
    int to = con.toId();
    con.unsetTo(); // make dangling
    d->dropOrInsert(geom.isZeroLength(con), con);
    removePointlessJunction(to);
    return true;
  } else {
    // removing middle segment => split into two dangling parts
    Connection con1;
    con1.setTo(con.to());
    con.unsetTo();
    QPolygon via1;
    while (via.size() > seg)
      via1.prepend(via.takeLast());
    con.setVia(via);
    con1.setVia(via1);

    if (geom.isZeroLength(con)) { // note that con is dangling by constr.
      d->drop(con);
      removePointlessJunction(con.fromId());
    } else {
      d->insert(con);
    }

    if (geom.isZeroLength(con1)) { // note that con1 is dangling by constr.
      removePointlessJunction(con1.toId());
    } else {
      d->insert(con1);
    }

    return true;
  }
}

bool CircuitMod::removeConnectionsEquivalentTo(int id) {
  Connection con(d->circ.connection(id));
  PinID from = con.from();
  PinID to = con.to();
  bool dang = con.isDangling();
  QPolygon via = con.via();
  QPolygon rvia;
  for (QPoint p: via)
    rvia.prepend(p);
  QSet<int> cc;
  for (auto const &c: d->circ.connections())
    if (c.id()!=id
        && ((c.from()==from && c.to()==to)
            || (c.from()==to && c.to()==from))
	&& (!dang || c.via()==via || c.via()==rvia))
      cc << c.id();

  bool res = false;
  for (int c: cc)
    if (deleteConnection(c))
      res = true;
  
  return res;
}

bool CircuitMod::removeIfDangling(int id) {
  if (d->circ.connections().contains(id) && d->circ.connection(id).isDangling()) 
    return deleteConnection(id);
  else
    return false;
}

bool CircuitMod::removeAllDanglingOrInvalid() {
  bool any = false;
  while (true) {
    /* This odd while loop is needed, because deleting a connection
       can effectively change IDs of other connections through
       deletion of a junction. */
    bool now = false;
    for (auto const &c: d->circ.connections())
      if (!c.isValid() || c.isDangling()) {
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

bool CircuitMod::removeIfInvalid(int id) {
  if (d->circ.connections().contains(id)
      && !d->circ.connection(id).isValid())
    return deleteConnection(id);
  else
    return false;
}

bool CircuitMod::simplifySegment(int id, int seg) {
  if (!d->circ.connections().contains(id))
    return false;
  Connection con(d->circ.connection(id));

  Geometry geom(d->circ, d->lib);
  QPolygon path(geom.connectionPath(con));
  int N = path.size();
  bool forcefirst = seg <= 0;
  bool forcenext = seg >= N-2;
  
  if (seg <= 0)
    seg = 1;
  if (seg >= N - 2)
    seg = N - 3;
  
  if (seg > 0) {
    // We have a segment that is neither first nor last
    // let's do something
    QPoint p0 = path[seg - 1];
    QPoint p1 = path[seg];
    QPoint p2 = path[seg + 1];
    QPoint p3 = path[seg + 2];
    if (p1.x() == p2.x()) {
      // Vertical segment
      int dx1 = p1.x() - p0.x();
      int dx3 = p3.x() - p2.x();
      // Move us, eliminate shortest neighbor. I think this works for both
      // zigzag and U-turn.
      // (Actual removal happens later through simplifiedPath
      bool rmnext = forcenext || abs(dx1) <= abs(dx3);
      if (forcefirst)
        rmnext = false;
      if (rmnext) {
        p1.setX(p3.x());
        p2.setX(p3.x());
      } else {
        p1.setX(p0.x());
        p2.setX(p0.x());
      }
      path[seg] = p1;
      path[seg + 1] = p2;
    } else if (p1.y() == p2.y()) {
      // Horizontal segment
      int dy1 = p1.y() - p0.y();
      int dy3 = p3.y() - p2.y();
      bool rmnext = forcenext || abs(dy1) <= abs(dy3);
      if (forcefirst)
        rmnext = false;
      if (rmnext) {
        p1.setY(p3.y());
        p2.setY(p3.y());
      } else {
        p1.setY(p0.y());
        p2.setY(p0.y());
      }
      path[seg] = p1;
      path[seg + 1] = p2;
    } else {
      // Diagonal segment. Make angled
      if (p1.y() == p0.y()) 
        // preceding segment is horizontal
        path.insert(seg + 1, QPoint(p2.x(), p1.y()));
      else
        path.insert(seg + 1, QPoint(p1.x(), p2.y()));
    }
  } else {
    // we have a path with no more than two segments, let's deal with diags
    for (int seg = N - 2; seg >= 0; --seg) {
      QPoint p1 = path[seg];
      QPoint p2 = path[seg + 1];
      if (p1.x()!=p2.x() && p1.y()!=p2.y()) {
        // diagonal segment
        if ((seg>0 && path[0].y()==p1.y())
            || (seg < N - 2 && path[seg + 2].x()==p2.x())) 
          path.insert(seg + 1, QPoint(p2.x(), p1.y()));
        else
          path.insert(seg + 1, QPoint(p1.x(), p2.y()));
      }
    }
  }
  
  path = Geometry::simplifiedPath(path);
  con.setVia(geom.viaFromPath(con, path));

  if (con.isDangling() && geom.isZeroLength(con)) {
    qDebug() << "Dropping connection during simplifySegment. Hmmm.";
    d->drop(con);
    /* Is this be dangerous if we are called from
       SceneConnection::mouseDoubleClickEvent()? */
  } else {
    d->insert(con);
    adjustOverlappingConnections(con.id());
  }
  return true;
}

bool CircuitMod::simplifyConnection(int id) {
  if (!d->circ.connections().contains(id))
    return false;
  Connection con(d->circ.connection(id));
  Geometry geom(d->circ, d->lib);
  QPolygon path0(geom.connectionPath(con));
  QPolygon path1 = Geometry::simplifiedPath(path0);
  if (path1.size() == path0.size())
    return false;
  con.setVia(geom.viaFromPath(con, path1));
  if (con.isDangling() && geom.isZeroLength(con))
    d->drop(con);
  else
    d->insert(con);
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
	if (con.isValid())
	  insert(con);
	else
	  drop(con);
      } 
      if (con.toId()==j) {
	con.setToId(id);
	if (con.isValid())
	  insert(con);
	else
	  drop(con);
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
  insert(j);
  insert(c);

  // Reroute both original connections to start at joint.
  a.setFrom(j.id(), "");
  b.setFrom(j.id(), "");
  a.setVia(patha);
  b.setVia(pathb);
  insert(a);
  insertOrDrop(b.isValid(), b);

  // The original starting point may have become a useless junction, so:
  removePointlessJunction(c.fromId());

  removeOverlappingJunctions(j.id());
  if (removePointlessJunction(j.id()))
    aelts.remove(j.id()); // unusual case: we previously inserted j, so
  // now we can simply remove it.
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
  if (!d->circ.connections().contains(id))
    return res;

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
        d->insert(br);
	d->removeOverlap(a.id(), b.id(), over);
	keepgoing = true;
	res = true;
	break;
      }
      Connection ar = a.reversed();
      over = d->overlappingStart(ar, b);
      if (over) {
        d->insert(ar);
	d->removeOverlap(a.id(), b.id(), over);
	keepgoing = true;
	res = true;
	break;
      }
      over = d->overlappingStart(ar, br);
      if (over) {
        d->insert(ar);
        d->insert(br);
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
  d->insert(d->circ.connection(id).translated(dd));
  return true;
}

bool CircuitMod::translateElement(int id, QPoint dd) {
  if (!d->circ.elements().contains(id))
    return false;
  d->insert(d->circ.element(id).translated(dd));
  return true;
}

bool CircuitMod::reroute(int id, Circuit const &origcirc) {
  if (!d->circ.connections().contains(id))
    return false;
  Router router(d->lib);
  d->insert(router.reroute(id, origcirc, d->circ));
  return true;
}

bool CircuitMod::removeOverlappingJunctions(int eltid) {
  return d->removeOverlappingJunctions(eltid);
}

int CircuitMod::injectJunction(int conid, QPoint at) {
  return d->injectJunction(conid, at);
}

int CircuitModData::injectJunction(int conid, QPoint at) {
  Connection con = circ.connection(conid);
  if (!con.isValid())
    return -1;

  Geometry geom(circ, lib);
  QPolygon path = geom.connectionPath(con);
  Geometry::Intersection inter(geom.intersection(at, path));

  if (inter.pointnumber<0)
    return -1;
  
  if (con.danglingStart() && inter.pointnumber==0 && inter.delta.isNull()) {
    // Insert at start of dangling. This is easy.
    Element junc(Element::junction(path[inter.pointnumber]));
    QPolygon via = con.via();
    via.removeFirst();
    con.setVia(via);
    con.setFrom(junc.id());
    insert(con);
    insert(junc);
    qDebug() << "inject at start of dangling";
    return junc.id();
  }
  if (con.danglingEnd() && inter.pointnumber==path.size()-1
      && inter.delta.isNull()) {
    // Insert at end of dangling. This is easy.
    Element junc(Element::junction(path[inter.pointnumber]));
    QPolygon via = con.via();
    via.removeLast();
    con.setVia(via);
    con.setTo(junc.id());
    insert(con);
    insert(junc);
    qDebug() << "inject at end of dangling" << con.report() << junc.report();
    return junc.id();
  }

  Element junc(Element::junction(path[inter.pointnumber] + inter.delta));
  Connection con1;
  con1.setTo(con.to());
  con.setTo(junc.id());
  con1.setFrom(junc.id());
  QPolygon via = con.via();
  int seg = inter.pointnumber;
  if (con.danglingStart())
    seg ++;
  // so seg==0 is always before via
  // con1 does not retain the intersection point as a via
  QPolygon via1 = via;
  for (int k=1; k<=seg; k++)
    if (!via1.isEmpty())
      via1.removeFirst();
  con1.setVia(via1);
  if (inter.delta.isNull()) {
    // con does not retain the intersection point as a via
    if (seg<1)
      via.clear();
    else
      via.resize(seg-1);
  } else {
    // con does retain the preceding point
    via.resize(seg);
  }
  con.setVia(via);
  insert(con);
  insert(con1);
  insert(junc);
  qDebug() << "inject somewhere else" << con.report() << con1.report();  
  return junc.id();
}

bool CircuitMod::rotateElements(QSet<int> eltids, int steps) {
  steps &= 3;
  Circuit c0 = d->circ;
  Geometry geom(d->circ, d->lib);

  int N = 0;
  QPoint p0;
  for (int id: eltids) {
    Element elt = d->circ.element(id);
    if (elt.isValid()) {
      p0 += geom.centerOfPinMass(elt);
      N++;
    }
  }
  p0 /= N; // original CM
  
  for (int id: eltids) {
    Element elt = d->circ.element(id);
    if (elt.isValid()) {
      elt.setRotation(elt.rotation() + steps);
      for (int k=0; k<steps; k++) {
	QPoint dp = elt.position() - p0;
	elt.setPosition(p0 + QPoint(dp.y(), -dp.x()));
      }
      d->insert(elt);
    }
  }
  QSet<int> cons = d->circ.connectionsIn(eltids);
  for (int id: cons) {
    Connection con = d->circ.connection(id);
    QPolygon via = con.via();
    for (QPoint &p: via) {
      for (int k=0; k<steps; k++) {
	QPoint dp = p - p0;
	p = p0 + QPoint(dp.y(), -dp.x());
      }
    }
    con.setVia(via);
    d->insert(con);
  }

  for (int c: (d->circ.connectionsFrom(eltids) + d->circ.connectionsTo(eltids))
	 - cons)
    reroute(c, c0);

  return false;
}

bool CircuitMod::rotateElement(int eltid, int steps) {
  if (!d->circ.elements().contains(eltid))
    return false;
  Circuit c0 = d->circ;
  Geometry geom(d->circ, d->lib);
  Element elt = d->circ.element(eltid);
  QPoint dx0 = geom.centerOfPinMass(elt);
  elt.setRotation(elt.rotation() + steps);
  QPoint dx1 = geom.centerOfPinMass(elt);
    elt.setPosition(elt.position() + dx0 - dx1);
  d->insert(elt);
  QSet<int> ee; ee << eltid;
  for (int c: d->circ.connectionsFrom(ee) + d->circ.connectionsTo(ee))
    reroute(c, c0);
  return true;
}

bool CircuitMod::flipElements(QSet<int> eltids) {
  Circuit c0 = d->circ;
  Geometry geom(d->circ, d->lib);

  int N = 0;
  QPoint p0;
  for (int id: eltids) {
    Element elt = d->circ.element(id);
    if (elt.isValid()) {
      p0 += geom.centerOfPinMass(elt);
      N++;
    }
  }
  p0 /= N; // original CM
  
  for (int id: eltids) {
    Element elt = d->circ.element(id);
    if (elt.isValid()) {
      elt.setFlipped(!elt.isFlipped());
      QPoint dp = elt.position() - p0;
      elt.setPosition(p0 + QPoint(-dp.x(), dp.y()));
      d->insert(elt);
    }
  }
  QSet<int> cons = d->circ.connectionsIn(eltids);
  for (int id: cons) {
    Connection con = d->circ.connection(id);
    QPolygon via = con.via();
    for (QPoint &p: via) {
      QPoint dp = p - p0;
      p = p0 + QPoint(-dp.x(), dp.y());
    }
    con.setVia(via);
    d->insert(con);
  }

  for (int c: (d->circ.connectionsFrom(eltids) + d->circ.connectionsTo(eltids))
	 - cons)
    reroute(c, c0);

  return false;
}

bool CircuitMod::flipElement(int eltid) {
  if (!d->circ.elements().contains(eltid))
    return false;
  Circuit c0 = d->circ;
  Geometry geom(d->circ, d->lib);
  Element elt = d->circ.element(eltid);
  elt.setFlipped(!elt.isFlipped());
  d->insert(elt);
  QSet<int> ee; ee << eltid;
  for (int c: d->circ.connectionsFrom(ee) + d->circ.connectionsTo(ee))
    reroute(c, c0);
  return true;
}
