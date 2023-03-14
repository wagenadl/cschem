// FileIO.h

#ifndef PCBFILEIO_H

#define PCBFILEIO_H

#include "Layout.h"

namespace PCBFileIO {
  Layout loadLayout(QString fn);
  bool saveLayout(QString fn, Layout const &); // true unless failed
};

#endif
