// Router.cpp

#include "Router.h"
#include "file/Connection.h"
#include "file/Element.h"
#include "svg/PartLibrary.h"
#include "file/Circuit.h"
#include <QDebug>

Router::Router(PartLibrary const *lib): lib(lib) {
}

QPointF Router::pinPosition(Element const &elt, QString pin) const {
  Part const &part = lib->part(elt.symbol());
  QPointF pos = elt.position();
  return lib->scale() * pos + part.shiftedPinPosition(pin);
}

QRectF Router::elementBBox(Element const &elt) const {
  Part const &part = lib->part(elt.symbol());
  QPointF pos = elt.position();
  qDebug() << "ebbox" << pos << part.shiftedBBox();
  return part.shiftedBBox().translated(lib->scale() * pos);
}

static QPolygon simplify(QPoint p0, QPolygon lst, QPoint pn) {
  lst.prepend(p0);
  lst.append(pn);
  int n = 1;
  while (n<lst.size() - 1) {
    if ((lst[n-1].x()==lst[n].x() && lst[n+1].x()==lst[n].x())
        || (lst[n-1].y()==lst[n].y() && lst[n+1].y()==lst[n].y()))
      lst.removeAt(n);
    else
      n++;
  }
  lst.removeFirst();
  lst.removeLast();
  return lst;
}

Connection Router::reroute(int conid,
                           Circuit const &origcirc,
                           Circuit const &newcirc) const {
  Connection con = newcirc.connections()[conid];

  Element const &origFrom = origcirc.elements()[con.fromId()];
  Element const &newFrom = newcirc.elements()[con.fromId()];
  Element const &origTo = origcirc.elements()[con.toId()];
  Element const &newTo = newcirc.elements()[con.toId()];

  QPoint origStart = lib->downscale(pinPosition(origFrom, con.fromPin()));
  QPoint origEnd = lib->downscale(pinPosition(origTo, con.toPin()));
  QPoint newStart = lib->downscale(pinPosition(newFrom, con.fromPin()));
  QPoint newEnd = lib->downscale(pinPosition(newTo, con.toPin()));

  QPolygon via = con.via();

  if (via.isEmpty()) {
    // might have to create elbow
    QPoint delta = newEnd - newStart;
    if (delta.x() && delta.y()) {
      QPoint center = (QPointF(newEnd + newStart)/2).toPoint();
      QPoint odelta = origEnd - origStart;
      if (abs(odelta.x()) < abs(odelta.y())) {
        // it was *originally* a (more-or-less) vertical line
        via << QPoint(newStart.x(), center.y());
        via << QPoint(newEnd.x(), center.y());
      } else {
        via << QPoint(center.x(), newStart.y());
        via << QPoint(center.x(), newEnd.y());
      }
      con.setVia(via);
    }
  } else {
    // Move first corner as needed
    if (via.first().x() == origStart.x()) 
      // was vertically aligned, so preserve that
      via.first().setX(newStart.x());
    else if (via.first().y() == origStart.y())
      // was horizontally aligned
      via.first().setY(newStart.y());
    // Move last corner as needed
    if (via.last().x() == origEnd.x()) 
      // was vertically aligned, so preserve that
      via.last().setX(newEnd.x());
    else if (via.last().y() == origEnd.y())
      // was horizontally aligned
      via.last().setY(newEnd.y());
    con.setVia(simplify(newStart, via, newEnd));
  }
  return con;
}

QPoint Router::awayFromCM(Part const &part, QString pin) const {
  QPointF delta(part.shiftedPinPosition(pin) - part.shiftedBBox().center());
  qDebug() << "cmdif" << part.name() << pin << delta;
  if (abs(delta.x()) > abs(delta.y()))
    return QPoint(delta.x() > 0 ? 1 : -1, 0);
  else
    return QPoint(0, delta.y() > 0 ? 1 : -1);
}

#define SIPHorizontal 1
#define SIPVertical 2

int Router::isSIP(Part const &part) const {
  bool samex = true;
  bool samey = true;
  for (auto pin: part.pinNames()) {
    QPointF p = part.shiftedPinPosition(pin);
    if (p.x() != 0)
      samex = false;
    if (p.y() != 0)
      samey = false;
  }
  if (samex)
    return SIPVertical;
  else if (samey)
    return SIPHorizontal;
  else
    return 0;
}

static double L2(QPointF p) {
  return p.x()*p.x() + p.y()*p.y();
}

QString Router::nearestNeighbor(Part const &part, QString pin) const {
  QStringList pins = part.pinNames();
  QString nearest;
  double dd = 1e9;
  QPointF p0 = part.shiftedPinPosition(pin);
  for (auto p: pins) {
    if (p!=pin) {
      double delta = L2(part.shiftedPinPosition(p) - p0);
      if (delta < dd) {
	dd = delta;
	nearest = p;
      }
    }
  }
  return nearest;
}

QPoint Router::preferredDirection(class Element const &elt, QString pin) const {
  Part const &part = lib->part(elt.symbol());
  QStringList pins = part.pinNames();
  switch (pins.size()) {
  case 0: // shouldn't happen
    return QPoint(1,0); // arbitrary
  case 1: // a port
    return awayFromCM(part, pin);
  case 2: // a two-pin device, e.g., resistor
    return awayFromCM(part, pin);
  case 3: // complex case. could be opamp, 7805, or SIP
    switch (isSIP(part)) {
    case SIPVertical:
      return QPoint(part.shiftedBBox().center().x() < 0 ? 1 : -1, 0);
    case SIPHorizontal:
      return QPoint(0, part.shiftedBBox().center().y() < 0 ? 1 : -1);
    default:
      return awayFromCM(part, pin);
    } break;
  default:
    switch (isSIP(part)) {
    case SIPVertical:
      return QPoint(part.shiftedBBox().center().x() < 0 ? 1 : -1, 0);
    case SIPHorizontal:
      return QPoint(0, part.shiftedBBox().center().y() < 0 ? 1 : -1);
    default: {
      QPointF deltaCM = part.shiftedPinPosition(pin)
        - part.shiftedBBox().center();
      QPointF deltaNearest = part.shiftedPinPosition(pin)
	- part.shiftedPinPosition(nearestNeighbor(part, pin));
      if (abs(deltaNearest.x()) < abs(deltaNearest.y()))
	// pins are likely arranged in horizontal lines
	return QPoint(0, deltaCM.y() > 0 ? 1 : -1);
      else
	// pins are likely arranged in vertical lines
	return QPoint(deltaCM.x() > 0 ? 1 : -1, 0);
    } break;
  }
  }
  return QPoint(1, 0); // arbitrary. shouldn't be reached
}
