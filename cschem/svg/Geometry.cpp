// Geometry.cpp

#include "Geometry.h"

#include "circuit/Circuit.h"
#include "svg/SymbolLibrary.h"
#include "ui/Style.h"
#include <QDebug>
#include <QTransform>
#include <QMap>
#include <QFontMetricsF>
#include <math.h>

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
  QMap<QString, QPoint> pinPositions(Element const &elt) const;
  QPoint preferredRoutingDirection(int elt, QString pin) const;
  QPoint preferredRoutingDirection(Element const &elt, QString pin) const;
  QPolygon connectionPath(Connection const &con) const;
  QTransform symbolToCircuitTransformation(Element const &elt) const;
  QTransform symbolToSceneElementTransformation(Element const &elt) const;
  QTransform symbolToSceneTransformation(Element const &elt) const;
  QRectF elementBBox(Element const &elt) const;
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

QTransform GeometryData::symbolToSceneTransformation(Element const &elt) const {
  QTransform xf;
  double s = lib.scale();
  xf.translate(s*elt.position.x(), s*elt.position.y());
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

QMap<QString, QPoint> Geometry::pinPositions(int elt) const {
  if (d)
    return d->pinPositions(d->circ.elements[elt]);
  else
    return QMap<QString, QPoint>();
}

QMap<QString, QPoint> Geometry::pinPositions(Element const &elt) const {
  if (d)
    return d->pinPositions(elt);
  else
    return QMap<QString, QPoint>();
}


QPoint GeometryData::pinPosition(int eltid, QString pin) const {
  return pinPosition(circ.elements[eltid], pin);
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

QRectF annotationBox(QPointF p, QString txt) {
  QFont af(Style::annotationFont());
  QFontMetricsF fm(af);
  double virtualcenterdy = af.pixelSize() * .3;
  QRectF rf(fm.boundingRect(txt));
  rf.translate(p.x() - rf.width()/2, p.y() + virtualcenterdy);
  return rf;
}

QRectF textualBox(QPointF p, QString txt) {
  QFont af(Style::annotationFont());
  QFontMetricsF fm(af);
  double virtualcenterdy = af.pixelSize() * .3;
  QRectF rf(fm.boundingRect(txt));
  rf.translate(p.x(), p.y() + virtualcenterdy);
  return rf;
}

QRectF Geometry::visualBoundingRect(Element const &elt) const {
  Q_ASSERT(d);
  Symbol const &prt(d->lib.symbol(elt.symbol()));
  QTransform xf(d->symbolToSceneTransformation(elt));
  QRectF bb0(prt.shiftedBBox());
  QRectF bb = xf.mapRect(bb0);
  if (elt.nameVisible)
    bb |= annotationBox(d->lib.upscale(elt.position)
                        + QPointF(elt.namePosition),
                        elt.name);
  if (elt.valueVisible)
    bb |= annotationBox(d->lib.upscale(elt.position)
                        + QPointF(elt.valuePosition),
                        elt.value);
  return bb;
}

QRectF Geometry::visualBoundingRect() const {
  Q_ASSERT(d);
  QRectF r;
  for (Element const &elt: d->circ.elements)
    r |= visualBoundingRect(elt);
  for (Connection const &con: d->circ.connections) {
    for (QPoint p: con.via) {
      QPointF pf(d->lib.upscale(p));
      QPointF dp(1e-3, 1e-3);
      r |= QRectF(pf - dp, pf + dp);
    }
  }
  for (Textual const &txt: d->circ.textuals)
    r |= textualBox(d->lib.upscale(txt.position), txt.text);
  return r;
}

QMap<QString, QPoint> GeometryData::pinPositions(Element const &elt) const {
  Symbol const &prt(lib.symbol(elt.symbol()));
  QMap<QString, QPoint> res;
  for (QString const &pin: prt.pinNames())
    res[pin] = pinPosition(elt, pin);
  return res;
}

QPoint GeometryData::pinPosition(Element const &elt, QString pin) const {
  Symbol const &prt(lib.symbol(elt.symbol()));
  QPointF pp = prt.shiftedPinPosition(pin);
  QTransform xf = symbolToCircuitTransformation(elt);
  QPoint p0 = xf.map(pp).toPoint();
  
  if (elt.flipped)
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
  qDebug() << "copm" << N << sum/N;
  if (N>0)
    return (sum/N).toPoint();

  for (Element const &elt: d->circ.elements) {
    Symbol const &symbol(d->lib.symbol(elt.symbol()));
    sum += d->elementBBox(elt).center();
    N ++;
  }
  qDebug() << "copm1" << N << sum/N;
  if (N>0)
    return (sum/N).toPoint();

  return QPoint(); // desperate
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
  QStringList pins = symbol.pinNames(); // these are presorted!
  QPointF sum;
  int N = pins.size();
  if (N==0)
    return symbol.shiftedBBox().center().toPoint();
  for (QString p: pins)
    sum += pinPosition(elt, p);
  double sumx = sum.x();
  double sumy = sum.y();
  double x = sum.x() / N;
  double y = sum.y() / N;
  if (x != round(x)) // attempt to break tie by weighing first pin more heavily
    x = (sumx + pinPosition(elt, pins[0]).x()) / (N+1);
  if (y != round(y))
    y = (sumy + pinPosition(elt, pins[0]).y()) / (N+1);
  return QPointF(x, y).toPoint();
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

QPoint Geometry::preferredRoutingDirection(class PinID const &pid) const {
  if (d)
    return d->preferredRoutingDirection(pid.element(), pid.pin());
  else
    return QPoint();
}

QPoint Geometry::preferredRoutingDirection(int eltid, QString pin) const {
  if (d)
    return d->preferredRoutingDirection(d->circ.elements[eltid], pin);
  else
    return QPoint();
}

QPoint Geometry::preferredRoutingDirection(Element const &elt, QString pin) const {
  if (d)
    return d->preferredRoutingDirection(elt, pin);
  else
    return QPoint();
}

QPoint GeometryData::preferredRoutingDirection(int eltid, QString pin) const {
  return preferredRoutingDirection(circ.elements[eltid], pin);
}

QPoint GeometryData::preferredRoutingDirection(Element const &elt,
                                               QString pin) const {
  Symbol const &prt(lib.symbol(elt.symbol()));
  QRectF bbox = elementBBox(elt);
  QPoint cm = bbox.center().toPoint();
  QPoint pinpos = pinPosition(elt, pin);
  QList<QPoint> otherpinpos;
  for (QString p: prt.pinNames())
    if (p!=pin)
      otherpinpos << pinPosition(elt, p);
  int leftmerit = 0;
  int rightmerit = 0;
  int upmerit = 0;
  int downmerit = 0;
  for (QPoint p: otherpinpos) {
    if (p.x()==pinpos.x()) {
      // avoid crashing through other pin
      if (p.y() > pinpos.y())
        downmerit -= 1000;
      else
        upmerit -= 1000;
    }
    if (p.y()==pinpos.y()) {
      // avoid crashing through other pin
      if (p.x() > pinpos.x())
        rightmerit -= 1000;
      else
        leftmerit -= 1000;
    }
  }
  int dx = cm.x() - pinpos.x();
  int dy = cm.y() - pinpos.y();
  rightmerit -= dx; // prefer to move away from cm
  leftmerit += dx;
  upmerit += dy;
  downmerit -= dy;
  QList<QPoint> dirs{QPoint(-1, 0), QPoint(1, 0), QPoint(0, -1), QPoint(0, 1)};
  QList<int> merits{leftmerit, rightmerit, upmerit, downmerit};
  QPoint bestdir;
  int merit = -1000000;
  for (int k=0; k<4; k++) {
    if (merits[k] > merit) {
      bestdir = dirs[k];
      merit = merits[k];
    }
  }
  return bestdir;
}

QRectF GeometryData::elementBBox(Element const &elt) const {
  Symbol const &prt(lib.symbol(elt.symbol()));
  QTransform xf(symbolToCircuitTransformation(elt));
  QRectF bb0(prt.shiftedBBox());
  QRectF bb = xf.mapRect(bb0);
  return bb;
}
