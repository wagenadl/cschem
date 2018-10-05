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
  Dim slotlength;
  Dim w, h;
  bool square;
  QString text;
  Orient orient;
  Dim fs;
  int arcangle;
  bool angleconstraint;
  EProps() {
    linewidth = Dim::fromMils(12);
    layer = Layer::Top;
    id = Dim::fromMils(40);
    od = Dim::fromMils(65);
    w = Dim::fromMils(65);
    h = Dim::fromMils(65);
    square = false;
    text = "";
    fs = Dim::fromMils(70);
    arcangle = 90;
    angleconstraint = false;
  }
};

#endif
