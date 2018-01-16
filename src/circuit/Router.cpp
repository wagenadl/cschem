// Router.cpp

#include "Router.h"
#include "circuit/Connection.h"
#include "circuit/Element.h"
#include "svg/PartLibrary.h"
#include "circuit/Circuit.h"
#include <QDebug>
#include "svg/Geometry.h"

Router::Router(PartLibrary const *lib): lib(lib) {
}

Connection Router::reroute(int conid,
                           Circuit const &origcirc,
                           Circuit const &newcirc) const {
  Geometry origgeom(origcirc, lib);
  Geometry newgeom(newcirc, lib);
  
  Connection con = newcirc.connection(conid);

  QPoint origStart = origgeom.pinPosition(con.from());
  QPoint origEnd = origgeom.pinPosition(con.to());
  QPoint newStart = newgeom.pinPosition(con.from());
  QPoint newEnd = newgeom.pinPosition(con.to());

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
    con.setVia(via);
    QPolygon path = newgeom.connectionPath(con);
    path = newgeom.simplifiedPath(path);
    con.setVia(newgeom.viaFromPath(con, path));
  }
  return con;
}
