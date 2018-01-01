// CircuitMod.cpp

#include "CircuitMod.h"
#include "Geometry.h"
#include "file/Circuit.h"

class CircuitModData {
public:
  CircuitModData(Circuit const &circ, PartLibrary const *lib):
    circ(circ), lib(lib) {
  }
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

void CircuitMod::deleteElement(int id) {
  Geometry geom(d->circ, d->lib);
  for (auto &c: d->circ.connections()) {
    if (c.fromId()==id) {
      c.setFromId(0); // make dangling
      c.via().prepend(geom.pinPosition(id, c.fromPin()));
      d->acons << c.id();
    }
    if (c.toId()==id) {
      c.setToId(0); // make dangling
      c.via().append(geom.pinPosition(id, c.toPin()));
      d->acons << c.id();
    }
  }

  d->aelts << id;
  d->circ.elements().remove(id);
}

bool CircuitMod::removePointlessJunction(int id) {
  QList<int> cc = d->circ.connectionsOn(id, "").toList();
  if (cc.size() > 2)
    return false;
  
  if (cc.size() == 2) {
    // reconnect what would be dangling
    Connection con1(d->circ.connection(cc[0]));
    Connection con2(d->circ.connection(cc[1]));

    if (con1.fromId() == id)
      con1.reverse();
    if (con2.toId() == id)
      con2.reverse();

    con1.setToId(con2.toId());
    con1.setToPin(con2.toPin());
    con1.via() += con2.via();
    d->circ.connections().remove(cc[1]);
    d->circ.connection(cc[0]) = con1;
  }

  for (int c: cc)
    d->acons << c;
  d->aelts << id;

  d->circ.elements().remove(id);

  return true;
}

void CircuitMod::deleteConnection(int id) {
  Connection con(d->circ.connection(id));

  d->acons << id;
  d->circ.connections().remove(id);
  
  int from = con.fromId();
  if (d->circ.element(from).type() == Element::Type::Junction)
    removePointlessJunction(from);

  int to = con.toId();
  if (d->circ.element(to).type() == Element::Type::Junction) 
    removePointlessJunction(to);
}
