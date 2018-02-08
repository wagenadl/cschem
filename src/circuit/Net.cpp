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
  void add(int con);
  void add(PinID const &pinid);
public:
  Circuit circ;
  QSet<int> connections;
  QSet<PinID> pins;
};

void NetData::add(int c) {
  if (connections.contains(c))
    return;
  connections << c;
  Connection const &con = circ.connections[c];
  add(con.from());
  add(con.to());
}

void NetData::add(PinID const &pinid) {
  if (pins.contains(pinid))
    return;
  pins << pinid;
  QSet<int> cc = circ.connectionsOn(pinid.element(), pinid.pin());
  for (int c: cc)
    add(c);
}

Net::Net(Circuit const &circ, PinID const &seedpin):
  d(new NetData(circ)) {
  d->add(seedpin);
}

Net::Net(Circuit const &circ, int seedelt, QString seedpin):
  Net(circ, PinID(seedelt, seedpin)) {
}
  
Net::Net(Circuit const &circ, int seedcon):
  d(new NetData(circ)) {
  d->add(seedcon);
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
