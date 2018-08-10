// Collector.h

#ifndef COLLECTOR_H

#define COLLECTOR_H

#include "data/Object.h"
#include <QMap>
#include <QList>

class Collector {
public:
  Collector(class Board const &);
  ~Collector();
  void collect(Group const &root);
public:
  QMap<Dim, QSet<Point>> const &holes() const; // key is ID
  QMap<Dim, QSet<Point>> const &roundHolePads() const; // key is OD
  QMap<Dim, QSet<Point>> const &squareHolePads() const; // key is OD
  QMap<Point, QSet<Point>> const &smdPads(Layer) const; // key is (W,H)
  QMap<Dim, QList<Trace>> const &traces(Layer) const; // key is LW
  QMap<Dim, QList<Arc>> const &arcs(Layer) const; // key is LW
  QMap<Dim, QList<Text>> const &texts(Layer) const; // key is FS
private:
  class ColData *d;
};

#endif
