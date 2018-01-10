// Geometry.cpp

#include "Geometry.h"

#include "file/Circuit.h"
#include "svg/PartLibrary.h"
#include <QDebug>
#include <QMap>

class OrderedPoint: public QPoint {
public:
  OrderedPoint(QPoint o) { x = o.x(); y = o.y(); }
  bool operator<(OrderedPoint const &o) const {
    if (x != o.x)
      return x < o.x;
    else
      return y < o.y;
  }
public:
  int x;
  int y;
};

class GeometryData {
public:
  GeometryData(Circuit const &circ, PartLibrary const *lib):
    circ(circ), lib(lib) {
  }
  void ensurePinDB();
  void ensureConnectionDB();
  QPoint pinPosition(int elt, QString pin) const;
  QPoint pinPosition(Element const &elt, QString pin) const;
  QPolygon connectionPath(Connection const &con) const;
public:
  Circuit circ;
  PartLibrary const *lib;
  mutable QMap<OrderedPoint, PinID> pindb;
  mutable QMap<OrderedPoint, int> condb;
};

Geometry::Geometry(Circuit const &circ, PartLibrary const *lib):
  d(new GeometryData(circ, lib)) {
}
Geometry::~Geometry() {
  delete d;
}

PinID Geometry::pinAt(QPoint p) const {
  d->ensurePinDB();
  if (d->pindb.contains(p))
    return d->pindb[p];
  else
    return PinID();
}

void GeometryData::ensurePinDB() {
  if (!pindb.isEmpty())
    return; // already done
  for (auto const &elt: circ.elements()) {
    Part const &prt(lib->part(elt.symbol()));
    if (!prt.isValid())
      continue;
    QStringList pins = prt.pinNames();
    for (QString p: pins) {
      QPoint pos = pinPosition(elt, p);
      pindb[pos] = PinID(elt.id(), p);
    }
  }
}

QPoint Geometry::pinPosition(class PinID const &pid) const {
  return d->pinPosition(pid.element(), pid.pin());
}

QPoint Geometry::pinPosition(int eltid, QString pin) const {
  return d->pinPosition(d->circ.element(eltid), pin);
}

QPoint Geometry::pinPosition(Element const &elt, QString pin) const {
  return d->pinPosition(elt, pin);
}

QPoint GeometryData::pinPosition(int eltid, QString pin) const {
  return pinPosition(circ.element(eltid), pin);
}

QPoint GeometryData::pinPosition(Element const &elt, QString pin) const {
  Part const &prt(lib->part(elt.symbol()));
  QPointF pp = prt.shiftedPinPosition(pin);
  if  (elt.isFlipped())
    pp = QPointF(-pp.x(), pp.y());
  for (int k=0; k<elt.rotation(); k++)
    pp = QPointF(pp.y(), -pp.x());
  return elt.position() + lib->downscale(pp);
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
  return d->connectionPath(d->circ.connection(conid));
}

QPolygon Geometry::connectionPath(Connection const &con) const {
  return d->connectionPath(con);
}

QPolygon GeometryData::connectionPath(Connection const &con) const {
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

int Geometry::connectionAt(QPoint p) const {
  d->ensureConnectionDB();
  if (d->condb.contains(p))
    return d->condb[p];
  else
    return -1;
}

void GeometryData::ensureConnectionDB() {
  if (!condb.isEmpty())
    return;

  for (auto &con: circ.connections()) {
    int id = con.id();
    QPolygon poly(connectionPath(con));
    int N = poly.size();
    for (int n=0; n<N-1; n++) {
      QPoint p0 = poly[n];
      QPoint p1 = poly[n+1];
      if (p0.x()!=p1.x() && p0.y()!=p1.y())
	continue; // skip diagonal segment
      if (p0.x()<p1.x()) {
	for (int x=p0.x(); x<=p1.x(); x++)
	  condb[QPoint(x, p0.y())] = id;
      } else if (p0.x()>p1.x()) {
	for (int x=p1.x(); x<=p0.x(); x++)
	  condb[QPoint(x, p0.y())] = id;
      } else if (p0.y()<p1.y()) {
	for (int y=p0.y(); y<=p1.y(); y++)
	  condb[QPoint(p0.x(), y)] = id;
      } else {
	for (int y=p1.y(); y<=p0.y(); y++)
	  condb[QPoint(p0.x(), y)] = id;
      }
    }
  }
}

Geometry::Intersection Geometry::intersection(QPoint p, QPolygon poly,
					      bool nodiag) {
  int N = poly.size();
  if (N<=0)
    return Intersection();

  Intersection best;
  int dist = 1000*1000*1000;
  
  for (int n=0; n<N-1; n++) {
    QPoint p0 = poly[n];
    QPoint p1 = poly[n+1];
    if (nodiag && p0.x()!=p1.x() && p0.y()!=p1.y())
      continue; // skip diagonal element
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
