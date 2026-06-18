// EProps.h

#ifndef EPROPS_H

#define EPROPS_H

#include "data/Dim.h"
#include "data/Arc.h"
#include "data/Layer.h"
#include <QString>

constexpr int DEFAULT_ARCANGLE = 180;

struct EProps {
  Dim linewidth;
  Layer layer;
  Dim od, id;
  Dim slotlength;
  Dim w, h;
  bool square;
  bool via;
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
    via = false;
    text = "";
    fs = Dim::fromMils(70);
    flip = false;
    arcangle = DEFAULT_ARCANGLE; 
    angleconstraint = false;
  }
};

#endif
