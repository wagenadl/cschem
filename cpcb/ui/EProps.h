// EProps.h

#ifndef EPROPS_H

#define EPROPS_H

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
  FreeRotation rota;
  bool flip;
  Dim fs;
  int arcangle; // here, we still allow -ve to mean starting at rota and
  // +ve to mean centered around rota
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
    flip = false;
    arcangle = 90;
    angleconstraint = false;
  }
};

#endif
