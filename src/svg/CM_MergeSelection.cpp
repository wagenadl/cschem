// CM_MergeSelection.cpp

#include "CircuitModData.h"

class CM_Merge {
public:
  CM_Merge(CircuitModData *d, QSet<int> sel):
    d(d),
    sel(sel),
    any(false),
    top(d->circ.subset(sel)),
    geom(top, d->lib) {
  }
  void considerPinPin();
  void considerPinCon();
  void considerConPin();
  void considerConCon();
public:
  CircuitModData *d;
  QSet<int> sel;
  bool any;
  Circuit top;
  Geometry geom;
  QSet<int> junctst;
};

bool CircuitMod::mergeSelection(QSet<int> sel) {
  CM_Merge mrg(d, sel);
  mrg.considerPinPin();
  mrg.considerPinCon();
  mrg.considerConPin();
  mrg.considerConCon();

  // Finally, cleanup
  for (int j: mrg.junctst) {
    removeOverlappingJunctions(j);
    removePointlessJunction(j);
  }

  return mrg.any;
}

void CM_Merge::considerPinPin() {
  // First, examine co-location of pins in BOTTOM (BASE) with pins in TOP
  for (Element const &eltbot: d->circ.elements()) {
    if (sel.contains(eltbot.id()))
      continue; // only look at elements _not_ in TOP
    Part const &prt(d->lib->part(eltbot.symbol()));
    if (!prt.isValid())
      continue;
    QStringList pins = prt.pinNames();
    for (QString p: pins) {
      PinID pidbot(eltbot.id(), p);
      QPoint pos = geom.pinPosition(eltbot, p);
      PinID pidtop = geom.pinAt(pos);
      if (pidtop.isValid()) { // gotcha
        if (Net(d->circ, pidtop).pins().contains(pidbot))
          continue; // The two are already connected
        junctst << d->addInPlaceConnection(pidbot, pidtop, pos);
        any = true;
      }
    }
  }
}

void CM_Merge::considerPinCon() {
  // Next, examine co-location of pins in BOTTOM with connections in TOP
  for (Element const &eltbot: d->circ.elements()) {
    if (sel.contains(eltbot.id()))
      continue; // only look at elements _not_ in TOP
    Part const &prt(d->lib->part(eltbot.symbol()));
    if (!prt.isValid())
      continue;
    QStringList pins = prt.pinNames();
    for (QString p: pins) {
      PinID pidbot(eltbot.id(), p);
      QPoint pos = geom.pinPosition(eltbot, p);
      int contop = geom.connectionAt(pos);
      if (contop>0) {
        // Let's just make sure the two aren't already connected:
        if (Net(d->circ, contop).pins().contains(pidbot))
          continue; // never mind        
        junctst << d->addInPlaceConnection(pidbot, contop, pos);
        any = true;
      }
    }
  }
}

void CM_Merge::considerConPin() {
  // Examine co-location of connection corners in BOTTOM with pins in
  // TOP. We don't have to study connection ends at pins; that's done
  // in considerPinPin.
  for (Connection const &conbot: d->circ.connections()) {
    for (QPoint pos: conbot.via()) {
      PinID pidtop = geom.pinAt(pos);
      if (pidtop.isValid()) { // gotcha
        if (Net(d->circ, pidtop).connections().contains(conbot.id()))
          continue; // The two are already connected
        junctst << d->addInPlaceConnection(pidtop, conbot.id(), pos);
        any = true;
      }
    }
  }
}

void CM_Merge::considerConCon() {
  // Examine co-location of connection corners in BOTTOM with
  // connections in TOP. We don't have to study connection ends at
  // pins; that's done in considerPinCon.
  for (Connection const &conbot: d->circ.connections()) {
    for (QPoint pos: conbot.via()) {
      int contop = geom.connectionAt(pos);
      if (contop>0) { // gotcha
        if (Net(d->circ,contop).connections().contains(conbot.id()))
          continue; // The two are already connected
        junctst << d->addInPlaceConnection(conbot.id(), contop, pos);
        any = true;
      }
    }
  }
}

int CircuitModData::addInPlaceConnection(int con0, int con1, QPoint pos) {
  int jid0 = injectJunction(con0, pos);
  if (jid0<0)
    return -1;
  int jid1 = injectJunction(con1, pos);
  if (jid1<0)
    return -1;
  insert(Connection(PinID(jid0), PinID(jid1)));
  removeOverlappingJunctions(jid0); // should remove jid1
  if (circ.elements().contains(jid1)) {
    qDebug() << "jid1 not removed. bad stuff may happen.";
  }
  return jid0;
}

int CircuitModData::addInPlaceConnection(PinID pidbot, int conid, QPoint pos) {
  /* Create a connection between a pins and a connection in the same location. */
  
  int jid = injectJunction(conid, pos);
  if (jid<0)
    return -1;
  qDebug() << "injected junction" << jid << "into" << circ.connection(conid).report();
  for (int c: circ.connectionsOn(PinID(jid))) 
    qDebug() << "connections onto our junction" << circ.connection(c).report();
  Element const &eltbot(circ.element(pidbot.element()));
  if (eltbot.type()!=Element::Type::Junction) {
    // rewire any connections to old pin to our new connection
    for (int c: circ.connectionsOn(pidbot)) 
      qDebug() << "will rewire" << circ.connection(c).report();
    rewire(circ.connectionsOn(pidbot), pidbot, PinID(jid));
  }
  insert(Connection(PinID(jid), pidbot));
  for (int c: circ.connectionsOn(PinID(jid))) 
    qDebug() << "connections onto our junction" << circ.connection(c).report();
  return jid;
}

int CircuitModData::addInPlaceConnection(PinID pidbot, PinID pidtop,
                                         QPoint pos) {
  /* Create a connection between two pins in the same location. */
  int jid = -1;
  Element const &elttop(circ.element(pidtop.element()));
  Element const &eltbot(circ.element(pidbot.element()));
  if (eltbot.type()==Element::Type::Junction
      || elttop.type()==Element::Type::Junction) {
    // At least one junction, so we'll connect the two and rewire
    // old connections from any non-junction.
    if (eltbot.type()==Element::Type::Junction) 
      jid = eltbot.id();
    else 
      // Bottom is not a junction -> rewire its previous connections
      rewire(circ.connectionsOn(pidbot), pidbot, pidtop);
	  
    if (elttop.type()==Element::Type::Junction) 
      jid = elttop.id();
    else 
      // Top is not a junction -> rewire its previous connections
      rewire(circ.connectionsOn(pidtop), pidtop, pidbot);

    // Now, connect top and bottom
    insert(Connection(pidtop, pidbot));
  } else {
    // Neither is a junction. If either has any connections, we'll need
    // to inject a junction
    QSet<int> cctop = circ.connectionsOn(pidtop);
    QSet<int> ccbot = circ.connectionsOn(pidbot);
    if (cctop.isEmpty() && ccbot.isEmpty()) {
      // Easy, just connect the two
      insert(Connection(pidtop, pidbot));
    } else {
      Element junc(Element::junction(pos));
      PinID pidj(junc.id());
      // Rewire old connections to either top or bottom pin
      rewire(cctop, pidtop, pidj);
      rewire(ccbot, pidbot, pidj);
      insert(Connection(pidtop, pidj));
      insert(Connection(pidbot, pidj));
      insert(junc);
      jid = junc.id();
    }
  }
  return jid;
}


