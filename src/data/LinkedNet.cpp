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

void LinkedNet::report() {
  QStringList pins;
  for (Nodename n: nodes)
    pins << n.toString();
  qDebug() << "LinkedNet" << name << ":" << pins.join(", ");
}
