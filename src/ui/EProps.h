// EProps.h

#ifndef EPROPS_H

#define EPROPS_H
#include "data/Orient.h"
#include "data/Dim.h"
#include "data/Arc.h"
#include "data/Layer.h"
#include <QString>

struct EProps {
  Dim linewidth;
  Layer layer;
  Dim od, id;
  Dim w, h;
  bool square;
  QString text;
  Orient orient;
  Dim fs;
  int arcangle;
};

#endif
