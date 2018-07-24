// LayerPoint.h

#ifndef LAYERPOINT_H

#define LAYERPOINT_H

#include "Layer.h"
#include "Point.h"

class LayerPoint {
public:
  LayerPoint() { layer=Layer::Invalid; }
  LayerPoint(Layer const &l, Point const &p): layer(l), point(p) { }
  bool operator==(LayerPoint const &o) const {
    return layer==o.layer && point==o.point;
  }
  bool operator<(LayerPoint const &o) const {
    if (int(layer)<int(o.layer))
      return true;
    else if (int(layer)>int(o.layer))
      return false;
    else if (point.x<o.point.x)
      return true;
    else if (point.x>o.point.x)
      return false;
    else
      return point.y<o.point.y;
  }
public:
  Layer layer;
  Point point;
};

inline uint qHash(LayerPoint const &lp) {
  uint hsh = int(lp.layer);
  return 2347891231*hsh ^ qHash(lp.point);
}

#endif
