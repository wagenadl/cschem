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
private:
  class PartLibrary const *lib;  
};

#endif
