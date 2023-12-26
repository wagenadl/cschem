// Collector.h

#ifndef COLLECTOR_H

#define COLLECTOR_H

#include "data/Object.h"
#include "FontSpec.h"
#include <QMap>
#include <QList>

class Collector {
public:
  Collector(class Board const &);
  ~Collector();
  void collect(Group const &root);
public:
  QMap<Dim, QList<Hole>> const &holes() const; // key is ID
  QMap<Dim, QList<NPHole>> const &npHoles() const; // key is D
  QMap<Dim, QList<Hole>> const &roundHolePads(Layer) const; // key is OD
  QMap<Dim, QList<Hole>> const &squareHolePads(Layer) const; // key is OD
  QMap<Point, QList<Pad>> const &smdPads(Layer) const; // key is (W,H)
  QMap<Dim, QList<Trace>> const &traces(Layer) const; // key is LW
  QMap<Dim, QList<Arc>> const &arcs(Layer) const; // key is LW
  QList<Polyline> const &filledPlanes(Layer) const;
  QMap<Gerber::FontSpec, QList<Text>> const &texts(Layer) const;
  QMap<Dim, Dim> const &roundWithNoClear() const; // key is plain OD, ...
  QMap<Dim, Dim> const &squareWithNoClear() const; // ... val is augmented
  QMap<Point, Point> const &smdWithNoClear() const; // sim.
private:
  class ColData *d;
};

#endif
