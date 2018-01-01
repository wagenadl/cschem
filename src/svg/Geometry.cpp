// Geometry.cpp

#include "Geometry.h"

#include "file/Circuit.h"
#include "svg/PartLibrary.h"

class GeometryData {
public:
  GeometryData(Circuit const &circ, PartLibrary const *lib):
    circ(circ), lib(lib) {
  }
public:
  Circuit circ;
  PartLibrary const *lib;
};

Geometry::Geometry(Circuit const &circ, PartLibrary const *lib):
  d(new GeometryData(circ, lib)) {
}
Geometry::~Geometry {
  delete d;
}

QPoint Geometry::pinPosition(int elt, QString pin) const {
}

QPolygon Geometry::connectionPath(int con) const {
}
