// Geometry.cpp

#include "Geometry.h"

#include "circuit/Circuit.h"
#include "svg/SymbolLibrary.h"
#include <QDebug>
#include  <QTransform>
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
  GeometryData(Circuit const &circ, SymbolLibrary const &lib):
    circ(circ), lib(lib) {
  }
  void ensurePinDB();
  void ensureConnectionDB();
  QPoint pinPosition(int elt, QString pin) const;
  QPoint pinPosition(Element const &elt, QString pin) const;
  QPolygon connectionPath(Connection const &con) const;
  QTransform symbolToCircuitTransformation(Element const &elt) const;
  QTransform symbolToSceneElementTransformation(Element const &elt) const;
public:
  Circuit circ;
  SymbolLibrary lib;
  mutable QMap<OrderedPoint, PinID> pindb;
  mutable QMap<OrderedPoint, int> condb;
};

Geometry::Geometry(Circuit const &circ, SymbolLibrary const &lib):
  d(new GeometryData(circ, lib)) {
}

Geometry::Geometry() {
  d = 0;
}

Geometry::Geometry(Geometry &&o) {
  d = o.d;
  o.d = 0;
}

Geometry &Geometry::operator=(Geometry &&o) {
  d = o.d;
  o.d = 0;
  return *this;
}

Geometry::~Geometry() {
  delete d;
}

QTransform GeometryData::symbolToCircuitTransformation(Element const &elt)
  const {
  QTransform xf;
  xf.translate(elt.position.x(), elt.position.y());
  double s = lib.scale();
  xf.scale(1/s, 1/s);
  xf.rotate(-elt.rotation*90);
  if (elt.flipped)
    xf.scale(-1, 1);
  return xf;
}

QTransform GeometryData::symbolToSceneElementTransformation(Element const &elt)
  const {
  QTransform xf;
  xf.rotate(-elt.rotation*90);
  if (elt.flipped)
    xf.scale(-1, 1);
  return xf;
}

PinID Geometry::pinAt(QPoint p) const {
  if (!d)
    return PinID();
  d->ensurePinDB();
  if (d->pindb.contains(p))
    return d->pindb[p];
  else
    return PinID();
}

void GeometryData::ensurePinDB() {
  if (!pindb.isEmpty())
    return; // already done
  for (auto const &elt: circ.elements) {
    Symbol const &prt(lib.symbol(elt.symbol()));
    if (!prt.isValid())
      continue;
    QStringList pins = prt.pinNames();
    for (QString p: pins) {
      QPoint pos = pinPosition(elt, p);
      pindb[pos] = PinID(elt.id, p);
    }
  }
}

QPoint Geometry::pinPosition(class PinID const &pid) const {
  if (d)
    return d->pinPosition(pid.element(), pid.pin());
  else
    return QPoint();
}

QPoint Geometry::pinPosition(int eltid, QString pin) const {
  if (d)
    return d->pinPosition(d->circ.elements[eltid], pin);
  else
    return QPoint();
}

QPoint Geometry::pinPosition(Element const &elt, QString pin) const {
  if (d)
    return d->pinPosition(elt, pin);
  else
    return QPoint();
}

QPoint GeometryData::pinPosition(int eltid, QString pin) const {
  return pinPosition(circ.elements[eltid], pin);
}

QRect Geometry::boundingRect(int elt) const {
  if (d)
    return boundingRect(d->circ.elements[elt]);
  else
    return QRect();
}

QRectF Geometry::svgBoundingRect(Element const &elt) const {
  if (!d) {
    qDebug() << "Cannot calculate svgBoundingRect w/o data";
    return QRectF();
  }
  
  QTransform xf = d->symbolToSceneElementTransformation(elt);
  Symbol const &sym(d->lib.symbol(elt.symbol()));
  QRectF bb = sym.shiftedBBox();
  return xf.mapRect(bb);
}

QRectF Geometry::defaultAnnotationSvgBoundingRect(Element const &elt,
						  QString annotation) const {
  if (!d) {
    qDebug() << "Cannot calculate defaultAnnotationSvgBoundingRect w/o data";
    return QRectF();
  }
  
  QTransform xf = d->symbolToSceneElementTransformation(elt);
  Symbol const &sym(d->lib.symbol(elt.symbol()));
  QRectF r0 = sym.shiftedAnnotationBBox(annotation);
  if (r0.isEmpty()) {
    if (annotation=="value") {
      r0 = defaultAnnotationSvgBoundingRect(elt, "name");
      return r0.translated(0, d->lib.scale()*3);
    } else {
      QRectF bb = svgBoundingRect(elt);
      QRectF r
	= QRectF(QPointF(bb.bottomRight()) + d->lib.upscale(QPoint(1, 2)),
		 d->lib.scale()*QSizeF(5, 1));
      return r;
    }
  } else {
    return xf.mapRect(r0);
  }
}

QRect Geometry::boundingRect(Element const &elt) const {
  if (!d)
    return QRect();
  Symbol const &prt(d->lib.symbol(elt.symbol()));
  QRectF bb = prt.shiftedBBox();
  QTransform xf = d->symbolToCircuitTransformation(elt);
  bb = xf.mapRect(bb);
  bb.adjust(-0.5, -0.5, 0.5, 0.5);
  return bb.toRect();
}

QRect Geometry::boundingRect() const {
  QRect r;
  if (d)
    for (Element const &elt: d->circ.elements)
      r |= boundingRect(elt);
  return r;
}

QPoint GeometryData::pinPosition(Element const &elt, QString pin) const {
  Symbol const &prt(lib.symbol(elt.symbol()));
  QPointF pp = prt.shiftedPinPosition(pin);
  QTransform xf = symbolToCircuitTransformation(elt);
  QPoint p0 = xf.map(pp).toPoint();
  
  if  (elt.flipped)
    pp = QPointF(-pp.x(), pp.y());
  for (int k=0; k<elt.rotation; k++)
    pp = QPointF(pp.y(), -pp.x());
  QPoint p1 = elt.position + lib.downscale(pp);
  if (p1!=p0)
    qDebug() << "TRANSFORMATION MISMATCH" << p0 << p1 << elt.report();
  return p1;
}

QPoint Geometry::centerOfPinMass() const {
  if (!d)
    return QPoint();
  int N = 0;
  QPointF sum;
  for (Element const &elt: d->circ.elements) {
    Symbol const &symbol(d->lib.symbol(elt.symbol()));
    QStringList pins = symbol.pinNames();
    N += pins.size();
    for (QString p: pins)
      sum += pinPosition(elt, p);
  }
  return (sum/N).toPoint();
}

QPoint Geometry::centerOfPinMass(int eltid) const {
  if (d)
    return centerOfPinMass(d->circ.elements[eltid]);
  else
    return QPoint();
}

QPoint Geometry::centerOfPinMass(Element const &elt) const {
  if (!d)
    return QPoint();
  Symbol const &symbol(d->lib.symbol(elt.symbol()));
  QStringList pins = symbol.pinNames();
  QPointF sum;
  int N = pins.size();
  for (QString p: pins)
    sum += pinPosition(elt, p);
  return (sum/N).toPoint();
}

QPolygon Geometry::connectionPath(int conid) const {
  if (d)
    return d->connectionPath(d->circ.connections[conid]);
  else
    return QPolygon();
}

QPolygon Geometry::connectionPath(Connection const &con) const {
  if (d)
    return d->connectionPath(con);
  else
    return QPolygon();
}

QPolygon GeometryData::connectionPath(Connection const &con) const {
  QPolygon res;
  if (!con.isValid())
    return res;
  if (!con.danglingStart())
    res << pinPosition(con.fromId, con.fromPin);
  for (auto p: con.via)
    res << p;
  if (!con.danglingEnd())
    res << pinPosition(con.toId, con.toPin);
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
  if (d)
    return isZeroLength(d->circ.connections[conid]);
  else
    return false;
}

bool Geometry::isZeroLength(Connection const &con) const {
  if (!d)
    return false;
  if (!con.isValid())
    return true;
  QPolygon p = connectionPath(con);
  if (p.size() <= 1)
    return true;
  else
    return p.last() == p.first();
}  

int Geometry::connectionAt(QPoint p) const {
  if (!d)
    return -1;
  d->ensureConnectionDB();
  if (d->condb.contains(p))
    return d->condb[p];
  else
    return -1;
}

void GeometryData::ensureConnectionDB() {
  if (!condb.isEmpty())
    return;

  for (auto &con: circ.connections) {
    int id = con.id;
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

Geometry::Intersection Geometry::intersection(QPoint p, int con, bool nodiag) {
  return intersection(p, connectionPath(con), nodiag);
}

Geometry::Intersection Geometry::intersection(QPoint p, Connection const &con,
                                              bool nodiag) {
  return intersection(p, connectionPath(con), nodiag);
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
	best.index = n;
	best.q = p0;
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
    best.index = N-1;
    best.q = poly.last();
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
  if (d)
    return viaFromPath(d->circ.connections[con], path);
  else
    return QPolygon();
}
