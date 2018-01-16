// FileIO.h

#ifndef FILEIO_H

#define FILEIO_H

#include "Schem.h"

namespace FileIO {
  Schem loadSchematic(QString fn);
  bool saveSchematic(QString fn, Schem const &); // true unless failed
};

#endif
