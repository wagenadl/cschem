// Router.cpp

#include "Router.h"
#include "file/Connection.h"
#include "file/Element.h"
#include "svg/PartLibrary.h"
#include "file/Circuit.h"
#include <QDebug>

Router::Router(PartLibrary const *lib): lib(lib) {
}

QPoint Router::pinPosition(Element const &elt, QString pin) const {
  Part const &part = lib->part(elt.symbol());
  QPoint pos = elt.position();
  return lib->scale() * pos + part.pinPosition(pin) - part.origin();
}

QRectF Router::elementBBox(Element const &elt) const {
  Part const &part = lib->part(elt.symbol());
  QPoint pos = elt.position();
  qDebug() << "ebbox" << pos << part.shiftedBBox();
  return part.shiftedBBox().translated(lib->scale() * pos);
}

static QList<QPoint> simplify(QPoint p0, QList<QPoint> lst, QPoint pn) {
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

  QPoint origStart = pinPosition(origFrom, con.fromPin()) / lib->scale();
  QPoint origEnd = pinPosition(origTo, con.toPin()) / lib->scale();
  QPoint newStart = pinPosition(newFrom, con.fromPin()) / lib->scale();
  QPoint newEnd = pinPosition(newTo, con.toPin()) / lib->scale();

  QList<QPoint> via = con.via();

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
  QRect bb = part.shiftedBBox();
  QPoint delta(part.pinPosition(pin) - part.origin() - bb.center());
  qDebug() << "cmdif" << part.name() << pin << delta;
  if (abs(delta.x()) > abs(delta.y()))
    return QPoint(delta.x() > 0 ? 1 : -1, 0);
  else
    return QPoint(0, delta.y() > 0 ? 1 : -1);
}

int Router::isSIP(Part const &part) const {
  bool samex = true;
  bool samey = true;
  QPoint o = part.origin();
  int x = o.x();
  int y = o.y();
  for (auto pin: part.pinNames()) {
    QPoint p = part.pinPosition(pin);
    if (p.x() != x)
      samex = false;
    if (p.y() != y)
      samey = false;
  }
  if (samex)
    return 1;
  else if (samey)
    return 2;
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
  QPoint p0 = part.pinPosition(pin);
  for (auto p: pins) {
    if (p!=pin) {
      double delta = L2(part.pinPosition(p) - p0);
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
    case 1: // horizontally oriented SIP
      return QPoint(part.origin().x() > part.bbox().center().x() ? 1 : -1, 0);
    case 2: // vertically oriented SIP
      return QPoint(0, part.origin().y() > part.bbox().center().y() ? 1 : -1);
    default:
      return awayFromCM(part, pin);
    } break;
  default:
    switch (isSIP(part)) {
    case 1: // horizontally oriented SIP
      return QPoint(part.origin().x() > part.bbox().center().x() ? 1 : -1, 0);
    case 2: // vertically oriented SIP
      return QPoint(0, part.origin().y() > part.bbox().center().y() ? 1 : -1);
    default: {
      QPoint deltaCM = part.pinPosition(pin) - part.bbox().center();
      QPoint deltaNearest = part.pinPosition(pin)
	- part.pinPosition(nearestNeighbor(part, pin));
      if (abs(deltaNearest.x()) > abs(deltaNearest.y()))
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

Connection Router::autoroute(class Element const &from, QString fromPin,
			     class Element const &to, QString toPin,
			     class Circuit const &) const {
  
  QPoint startDir = preferredDirection(from, fromPin);
  QPoint endDir = preferredDirection(to, toPin);
  qDebug() << "ar from" << from.symbol() << from.id() << fromPin << startDir;
  qDebug() << "ar to" << to.symbol() << to.id() << toPin << endDir;

  // should take case where to is a junction separately
  // (right now, "from" cannot be a junction)
  
  QPoint fromP = pinPosition(from, fromPin) / lib->scale();
  QPoint toP = pinPosition(to, toPin) / lib->scale();
  QPoint midP = (fromP + toP) / 2;
  QList<QPoint> via;

  switch (QPoint::dotProduct(startDir, endDir)) {
  case 0:
    qDebug() << "t0" << startDir;
    // one horizontal, other vertical
    // this could be easy with a single corner, if there is space
    if (QPoint::dotProduct(toP - fromP, startDir) > 0
	&& QPoint::dotProduct(fromP - toP, endDir) > 0) {
      if (startDir.x()==0)
	via << QPoint(fromP.x(), toP.y());
      else
	via << QPoint(toP.x(), fromP.y());
    } else {
      // complicated
    }
    break;
  case -1:
    // pointing in opposite directions
    if (QPoint::dotProduct(toP-fromP, startDir) > 0) {
    qDebug() << "t-1 elbow" << startDir;
      // have space for an elbow
      if (startDir.y()==0)
	via << QPoint(midP.x(), fromP.y()) << QPoint(midP.x(), toP.y());
      else
	via << QPoint(fromP.x(), midP.y()) << QPoint(toP.x(), midP.y());
    } else {
      qDebug() << "t-1 circle" << startDir;
      // must route in a circle
      /* (Actually, it might be possible to go in between the elements,
	 but I haven't explored that yet.)
      */
      QRectF bb0 = elementBBox(from) | elementBBox(to);
      QRect bb
	= QRectF(bb0.topLeft() / lib->scale(), bb0.bottomRight() / lib->scale())
	.toRect();
      bb += QMargins(2, 2, 2, 2);
      qDebug() << elementBBox(from) << elementBBox(from) << bb;
      if (startDir.y()==0) {
	// horizontal orientation
	int dyTop = abs(bb.top() - fromP.y()) + abs(bb.top() - toP.y());
	int dyBot = abs(bb.bottom() - fromP.y()) + abs(bb.bottom() - toP.y());
	int midY = dyTop < dyBot ? bb.top() : bb.bottom();
	via << fromP + startDir;
	via << QPoint(fromP.x(), midY) + startDir;
	via << QPoint(toP.x(), midY) + endDir;
	via << toP + endDir;
      } else {
	// vertical orientation
	int dxleft = abs(bb.left() - fromP.y()) + abs(bb.left() - toP.y());
	int dxright = abs(bb.right() - fromP.y()) + abs(bb.right() - toP.y());
	int midX = dxleft < dxright ? bb.left() : bb.right();
	via << fromP + startDir;
	via << QPoint(midX, fromP.y()) + startDir;
	via << QPoint(midX, toP.y()) + endDir;
	via << toP + endDir;
      }
    }
    break;
  case 1: { // pointing in the same direction
    qDebug() << "t1" << startDir << midP;
    QPoint midP = (QPoint::dotProduct(toP-fromP, startDir) > 0) ? toP : fromP;
    if (startDir.y()==0) {
      // horizontal direction
      via << QPoint(midP.x(), fromP.y()) + startDir;
      via << QPoint(midP.x(), toP.y()) + startDir;
    } else {
      via << QPoint(fromP.x(), midP.y()) + startDir;
      via << QPoint(toP.x(), midP.y()) + startDir;
    }
  } break;
  }

  Connection c;
  c.setFromId(from.id());
  c.setFromPin(fromPin);
  c.setToId(to.id());
  c.setToPin(toPin);
  c.setVia(via);

  qDebug() << from.symbol() << from.id() << fromPin << fromP;
  qDebug() << "  " << via;
  qDebug() << "  " << to.symbol() << to.id() << toPin << toP;
  return c;
}
