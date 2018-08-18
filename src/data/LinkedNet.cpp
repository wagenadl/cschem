// LinkedNet.cpp

#include "LinkedNet.h"

LinkedNet::LinkedNet(Circuit const &circ, Net const &net) {
  for (PinID const &pin: net.pins()) {
    if (circ.elements.contains(pin.element())) {
      Element const &elt(circ.elements[pin.element()]);
      if (elt.type == Element::Type::Component) 
	nodes << Nodename(elt.name, pin.pin());
    }
  }
  name = net.name();
}

bool LinkedNet::containsMatch(Nodename const &n) const {
  for (Nodename const &x: nodes)
    if (x.matches(n))
      return true;
  return false;
}

MatchQuality LinkedNet::matchQuality(Nodename const &n) const {
  MatchQuality mq = MatchQuality::None;
  for (Nodename const &x: nodes) {
    MatchQuality mq1 = x.matchQuality(n);
    if (mq1>mq)
      mq = mq1;
  }
  return mq;
}
  

void LinkedNet::report() {
  QStringList pins;
  for (Nodename n: nodes)
    pins << n.toString();
  qDebug() << "LinkedNet" << name << ":" << pins.join(", ");
}
