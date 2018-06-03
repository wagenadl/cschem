// FileIO.h

#ifndef FILEIO_H

#define FILEIO_H

#include "Layout.h"

namespace FileIO {
  Layout loadLayout(QString fn);
  bool saveLayout(QString fn, Layout const &); // true unless failed
};

#endif
