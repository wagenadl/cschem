// FileIO.h

#ifndef FILEIO_H

#define FILEIO_H

#include "Schem.h"

namespace FileIO {
  Schem loadSchematic(QString fn);
  void saveSchematic(QString fn, Schem const &);
};

#endif
