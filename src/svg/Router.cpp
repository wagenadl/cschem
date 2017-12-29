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
                           Circuit const &newcirc) {
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
