// Collector.h

#ifndef COLLECTOR_H

#define COLLECTOR_H

#include "data/Object.h"
#include "FontSpec.h"
#include <QMap>
#include <QList>

class Collector {
public:
  struct PadInfo {
    Point p;
    bool fpcon;
    bool noclear;
  };
public:
  Collector(class Board const &);
  ~Collector();
  void collect(Group const &root);
public:
  QMap<Dim, QSet<Point>> const &holes() const; // key is ID
  QMap<Dim, QList<PadInfo>> const &roundHolePads(Layer) const; // key is OD
  QMap<Dim, QList<PadInfo>> const &squareHolePads(Layer) const; // key is OD
  QMap<Point, QList<PadInfo>> const &smdPads(Layer) const; // key is (W,H)
  QMap<Dim, QList<Trace>> const &traces(Layer) const; // key is LW
  QMap<Dim, QList<Arc>> const &arcs(Layer) const; // key is LW
  QList<Polyline> const &filledPlanes(Layer) const;
  QMap<Gerber::FontSpec, QList<Text>> const &texts(Layer) const;
private:
  class ColData *d;
};

#endif
