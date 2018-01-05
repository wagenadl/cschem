// Geometry.cpp

#include "Geometry.h"

#include "file/Circuit.h"
#include "svg/PartLibrary.h"
#include <QDebug>

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

QPoint Geometry::pinPosition(class PinID const &pid) const {
  return pinPosition(pid.element(), pid.pin());
}

QPoint Geometry::pinPosition(int eltid, QString pin) const {
  return pinPosition(d->circ.element(eltid), pin);
}

QPoint Geometry::pinPosition(Element const &elt, QString pin) const {
  Part const &prt(d->lib->part(elt.symbol()));
  QPointF pp = prt.shiftedPinPosition(pin);
  for (int k=0; k<elt.rotation(); k++)
    pp = QPointF(pp.y(), -pp.x());
  return elt.position() + d->lib->downscale(pp);
}

QPoint Geometry::centerOfPinMass() const {
  int N = 0;
  QPointF sum;
  for (Element const &elt: d->circ.elements()) {
    Part const &part(d->lib->part(elt.symbol()));
    QStringList pins = part.pinNames();
    N += pins.size();
    for (QString p: pins)
      sum += pinPosition(elt, p);
  }
  return (sum/N).toPoint();
}

QPoint Geometry::centerOfPinMass(int eltid) const {
  return centerOfPinMass(d->circ.element(eltid));
}

QPoint Geometry::centerOfPinMass(Element const &elt) const {
  Part const &part(d->lib->part(elt.symbol()));
  QStringList pins = part.pinNames();
  QPointF sum;
  for (QString p: pins)
    sum += pinPosition(elt, p);
  return (sum/pins.size()).toPoint();
}

QPolygon Geometry::connectionPath(int conid) const {
  return connectionPath(d->circ.connection(conid));
}

QPolygon Geometry::connectionPath(Connection const &con) const {
  QPolygon res;
  if (!con.isValid())
    return res;
  if (!con.danglingStart())
    res << pinPosition(con.fromId(), con.fromPin());
  for (auto p: con.via())
    res << p;
  if (!con.danglingEnd())
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

bool Geometry::isZeroLength(int conid) const {
  return isZeroLength(d->circ.connection(conid));
}

bool Geometry::isZeroLength(Connection const &con) const {
  if (!con.isValid())
    return true;
  QPolygon p = connectionPath(con);
  qDebug() << "iZL" << con.report() << p;
  if (p.size() <= 1)
    return true;
  else
    return p.last() == p.first();
}  

Geometry::Intersection Geometry::intersection(QPoint p, QPolygon poly) {
  int N = poly.size();
  if (N<=0)
    return Intersection();

  Intersection best;
  int dist = 1000*1000*1000;
  
  for (int n=0; n<N-1; n++) {
    QPoint p0 = poly[n];
    QPoint p1 = poly[n+1];
    while (p0 != p1) {
      int d0 = (p - p0).manhattanLength();
      if (d0 < dist) {
	best.pointnumber = n;
	best.delta = p0 - poly[n];
	dist = d0;
      }
      if (p0.x() < p1.x())
	p0 += QPoint(1,0);
      else if (p0.x() > p1.x())
	p0 += QPoint(-1,0);
      else if (p0.y() < p1.y())
	p0 += QPoint(0,1);
      else if (p0.y() > p1.y())
	p0 += QPoint(0,-1);
    }
  }
  int d0 = (p - poly.last()).manhattanLength();
  if (d0 < dist) {
    best.pointnumber = N-1;
    best.delta = QPoint();
  }
  return best;
}

QPolygon Geometry::viaFromPath(class Connection const &con, QPolygon path) {
  if (!con.danglingStart())
    path.removeFirst();
  if (!con.danglingEnd())
    path.removeLast();
  return path;
}

QPolygon Geometry::viaFromPath(int con, QPolygon path) const {
  return viaFromPath(d->circ.connection(con), path);
}
