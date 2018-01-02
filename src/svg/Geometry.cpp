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
  return connectionPath(d->circ.connection(conid));
}

QPolygon Geometry::connectionPath(Connection const &con) const {
  QPolygon res;
  if (con.fromId()>0)
    res << pinPosition(con.fromId(), con.fromPin());
  for (auto p: con.via())
    res << p;
  if (con.toId()>0)
    res << pinPosition(con.toId(), con.toPin());
  return res;
}

QPolygon Geometry::simplifiedPath(QPolygon pp) {
  int n = 1;
  while (n<pp.size() - 1) {
    if ((pp[n-1].x()==pp[n].x() && pp[n+1].x()==pp[n].x()) // vertical cont.
        || (pp[n-1].y()==pp[n].y() && pp[n+1].y()==pp[n].y()) // hor. cont.
        || (pp[n].x()==pp[n+1].x() && pp[n].y()==pp[n+1].y())) // zero-length
      pp.removeAt(n);
    else
      n++;
  }
  
  if (pp.size()>2)
    if (pp[0].x()==pp[1].x() && pp[0].y()==pp[1].y())
      // this is a zero-length test not included in above loop
      pp.removeAt(0);
  return pp;
}
