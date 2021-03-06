// Router.cpp

#include "Router.h"
#include "circuit/Connection.h"
#include "circuit/Element.h"
#include "svg/SymbolLibrary.h"
#include "circuit/Circuit.h"
#include <QDebug>
#include "svg/Geometry.h"

Router::Router(SymbolLibrary const &lib): lib(lib) {
}

Connection Router::reroute(int conid,
                           Circuit const &origcirc,
                           Circuit const &newcirc) const {
  Geometry origgeom(origcirc, lib);
  Geometry newgeom(newcirc, lib);
  
  Connection con = newcirc.connections[conid];

  QPoint origStart = origgeom.pinPosition(con.from());
  QPoint origEnd = origgeom.pinPosition(con.to());
  QPoint newStart = newgeom.pinPosition(con.from());
  QPoint newEnd = newgeom.pinPosition(con.to());
  QPoint prefStartDir = newgeom.preferredRoutingDirection(con.from());
  QPoint prefEndDir = newgeom.preferredRoutingDirection(con.to());

  if (con.via.isEmpty()) {
    // might have to create elbow
    QPoint delta = newEnd - newStart;
    if (delta.x() && delta.y()) {
      QPoint center = (QPointF(newEnd + newStart)/2).toPoint();
      // we are not smart enough to avoid routing through element
      if (prefStartDir.x()) {
        if (prefEndDir.x()) {
          // route hor/vert/hor
          con.via << QPoint(center.x(), newStart.y());
          con.via << QPoint(center.x(), newEnd.y());
        } else {
          // route hor/vert
          con.via << QPoint(newEnd.x(), newStart.y());
        }
      } else {
        if (prefEndDir.y()) {
          // route vert/hor/vert
          con.via << QPoint(newStart.x(), center.y());
          con.via << QPoint(newEnd.x(), center.y());
        } else {
          // route vert/hor
          con.via << QPoint(newStart.x(), newEnd.y());
        }
      }
    }
  } else {
    // Move first corner as needed
    if (con.via.first().x() == origStart.x()) 
      // was vertically aligned, so preserve that
      con.via.first().setX(newStart.x());
    else if (con.via.first().y() == origStart.y())
      // was horizontally aligned
      con.via.first().setY(newStart.y());
    // Move last corner as needed
    if (con.via.last().x() == origEnd.x()) 
      // was vertically aligned, so preserve that
      con.via.last().setX(newEnd.x());
    else if (con.via.last().y() == origEnd.y())
      // was horizontally aligned
      con.via.last().setY(newEnd.y());
    QPolygon path = newgeom.connectionPath(con);
    path = newgeom.simplifiedPath(path);
    con.via = newgeom.viaFromPath(con, path);
  }
  return con;
}
