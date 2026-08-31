// NetExporter.h

#ifndef NETEXPORTER_H

#define NETEXPORTER_H

#include "circuit/Schem.h"

class NetExporter {
public:
  NetExporter(Schem const &s): s(s) {}
  bool exportNet(QString ofn) const;
private:
  Schem const &s;
};

#endif
