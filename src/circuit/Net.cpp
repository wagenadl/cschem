// Net.cpp

#include "Net.h"
#include "Circuit.h"
#include "Element.h"
#include "Connection.h"
#include <QSet>
#include <QDebug>

class NetData: public QSharedData {
public:
  NetData(Circuit const &circ): circ(circ) { }
  void addCon(int con);
  void addPin(PinID const &pinid);
  void determineName();
public:
  Circuit circ;
  QSet<int> connections;
  QSet<PinID> pins;
  QString name;
};

void NetData::addCon(int c) {
  if (connections.contains(c))
    return;
  connections << c;
  Connection const &con = circ.connections[c];
  addPin(con.from());
  addPin(con.to());
}

void NetData::addPin(PinID const &pinid) {
  if (pins.contains(pinid))
    return;
  pins << pinid;
  QSet<int> cc = circ.connectionsOn(pinid.element(), pinid.pin());
  for (int c: cc)
    addCon(c);
}

void NetData::determineName() {
  /* If possible, we'll use the name of a port as representative of
     our net. Otherwise, we'll use a "Ref:Pin"-style name.
     In either case, we'll use the alphabetically first candidate if
     there are multiple possibilities.
  */
  QStringList ports;
  QStringList alts;
  for (PinID const &id: pins) {
    if (circ.elements.contains(id.element())) {
      Element const &elt = circ.elements[id.element()];
      if (elt.type == Element::Type::Port) {
	if (elt.name.isEmpty())
	  ports << elt.symbol().split(":").last();
	else
	  ports << elt.name;
      } else if (elt.type == Element::Type::Component && !elt.name.isEmpty()) {
	alts << elt.name + ":" + id.pin();
      }
    }
  }
  if (!ports.isEmpty()) {
    ports.sort();
    name = ports.first();
  } else if (!alts.isEmpty()) {
    alts.sort();
    name = alts.first();
  } else {
    name = "";
  }
}

Net::Net(Circuit const &circ, PinID const &seedpin):
  d(new NetData(circ)) {
  d->addPin(seedpin);
  d->determineName();
}

Net::Net(Circuit const &circ, int seedelt, QString seedpin):
  Net(circ, PinID(seedelt, seedpin)) {
}
  
Net::Net(Circuit const &circ, int seedcon):
  d(new NetData(circ)) {
  d->addCon(seedcon);
}

Net::~Net() {
}

Net::Net(Net const &o) {
  d = o.d;
}

Net &Net::operator=(Net const &o) {
  d = o.d;
  return *this;
}

QSet<int> Net::connections() const {
  return d->connections;
}

QSet<PinID> Net::pins() const {
  return d->pins;
}

QString Net::name() const {
  return d->name;
}

QList<Net> Net::allNets(Circuit const &circ) {
  QSet<PinID> allPins;
  for (Connection const &c: circ.connections) {
    allPins << c.from();
    allPins << c.to();
  }
  QList<Net> nets;
  while (!allPins.isEmpty()) {
    PinID id = *allPins.begin();
    allPins.erase(allPins.begin());
    if (id.isValid()) {
      Net net(circ, id);
      nets << net;
      for (PinID p: net.pins())
	allPins.remove(p);
    }
  }
  return nets;
}
