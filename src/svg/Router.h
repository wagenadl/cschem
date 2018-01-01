// Router.h

#ifndef ROUTER_H

#define ROUTER_H

#include <QPoint>
#include "file/Connection.h"

class Router {
public:
  Router(class PartLibrary const *lib);
  Connection reroute(int conid, class Circuit const &origcirc,
                     class Circuit const &newcirc) const;
  QPointF pinPosition(class Element const &elt, QString pin) const;
  QPoint preferredDirection(class Element const &elt, QString pin) const;
  QRectF elementBBox(class Element const &elt) const;
private:
  QPoint awayFromCM(class Part const &part, QString pin) const;
  int isSIP(Part const &part) const;
  QString nearestNeighbor(Part const &part, QString pin) const;
private:
  class PartLibrary const *lib;  
};

#endif
