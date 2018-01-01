// Geometry.cpp

#include "Geometry.h"

#include "file/Circuit.h"
#include "svg/PartLibrary.h"

class GeometryData {
public:
  GeometryData(Circuit const &circ, PartLibrary const *lib):
    circ(circ), lib(lib) {
  }
public:
  Circuit circ;
  PartLibrary const *lib;
};

Geometry::Geometry(Circuit const &circ, PartLibrary const *lib):
  d(new GeometryData(circ, lib)) {
}
Geometry::~Geometry() {
  delete d;
}

QPoint Geometry::pinPosition(int eltid, QString pin) const {
  Element const &elt(d->circ.element(eltid));
  Part const &prt(d->lib->part(elt.symbol()));
  QPointF pp = prt.shiftedPinPosition(pin);
  return elt.position() + d->lib->downscale(pp);
}

QPolygon Geometry::connectionPath(int conid) const {
  QPolygon res;
  Connection const &con(d->circ.connection(conid));
  if (con.fromId()>0)
    res << pinPosition(con.fromId(), con.fromPin());
  for (auto p: con.via())
    res << p;
  if (con.toId()>0)
    res << pinPosition(con.toId(), con.toPin());
  return res;
}
