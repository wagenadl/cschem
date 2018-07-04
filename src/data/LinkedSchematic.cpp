// LinkedSchematic.cpp

#include "LinkedSchematic.h"
#include "../../cschem/src/circuit/Schem.h"
#include "../../cschem/src/file/FileIO.h"

class LSData {
public:
  LSData() {
  }
public:
  QString fn;
  Schem schem;
};

LinkedSchematic::LinkedSchematic(): d(new LSData) {
}

void LinkedSchematic::link(QString fn) {
  d->fn = fn;
  d->schem = FileIO::loadSchematic(fn);
}

void LinkedSchematic::unlink() {
  d->fn = "";
  d->schem = Schem();
}

LinkedSchematic::~LinkedSchematic() {
  delete d;
}

bool LinkedSchematic::isValid() const {
  return !d->fn.isEmpty();
}

Schem const &LinkedSchematic::schematic() const {
  return d->schem;
}

