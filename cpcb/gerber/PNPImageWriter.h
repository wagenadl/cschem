// PnpimageWriter.h

#ifndef PNPIMAGEWRITER_H

#define PNPIMAGEWRITER_H

#include "data/Dim.h"
#include <QString>

class PNPImageWriter {
public:
  PNPImageWriter();
  bool write(class Layout const &layout, class PickNPlace const &pnp,
             QString filename);
};

#endif
