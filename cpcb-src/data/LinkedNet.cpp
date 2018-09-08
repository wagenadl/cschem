// LinkedNet.cpp

#include "LinkedNet.h"
#include "svg/Symbol.h"

#include <QDebug>


LinkedNet::LinkedNet(Schem const &schem, Net const &net) {
  Circuit const &circ(schem.circuit());
  for (PinID const &pin: net.pins()) {
    int eid = pin.element();
    if (circ.elements.contains(eid)) {
      Element const &elt(circ.elements[eid]);
      QString ename = elt.name;
      QString pname = pin.pin();
      if (elt.type == Element::Type::Component) {
        int id1 = circ.containerOf(eid);
        if (id1>0) {
          // We wish to replace "A1.2:-" by "A1:6/2.-"
          Symbol const &sym(schem.symbolForElement(id1));
          int slotno = 1;
          int dotidx = ename.indexOf(".");
          if (dotidx>0)
            slotno = ename.mid(dotidx+1).toInt();
          if (slotno>0) {
            auto map = sym.containedPins(slotno);
            QString pinnum = map[pname];
            if (!pinnum.isEmpty()) {
              if (sym.slotCount()>1)
                pname = QString::number(slotno) + "." + pname;
              ename = ename.left(dotidx);
              pname = pinnum + "/" + pname;
            }
          }
        }
	nodes << Nodename(ename, pname);
      }
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

QDebug &operator<<(QDebug &dbg, LinkedNet const &lnet) {
  QStringList pins;
  for (Nodename n: lnet.nodes)
    pins << n.toString();
  dbg << lnet.name << ":" << pins.join(", ");
  return dbg;
}
