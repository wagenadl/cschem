// Router.h

#ifndef ROUTER_H

#define ROUTER_H

#include <QPoint>
#include "circuit/Connection.h"

class Router {
public:
  Router(class SymbolLibrary const &lib);
  Connection reroute(int conid, class Circuit const &origcirc,
                     class Circuit const &newcirc) const;
private:
  class SymbolLibrary const &lib;  
};

#endif
