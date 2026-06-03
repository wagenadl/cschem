// SegmentDB.h

#ifndef SEGMENTDB_H

#define SEGMENTDB_H

#include <QMultiHash>
#include <QPoint>
#include <QSet>

class SegmentDB {
public:
  struct Segment {
    int conid;
    int start;
    int end;
    Segment(int conid, int a, int b);
  };
public:
  SegmentDB();
  void insert(int conid, QPoint p1, QPoint p2);
  bool any() const { return _any; }
  int find(QPoint p) const;
  bool contains(QPoint p) const { return find(p) > 0; }
  SegmentDB restricted(QSet<int> cons,
                       bool inc_not_exc) const;
private:
  bool _any;
  QMultiHash<int, Segment> horizontals; // map y to (conid, x1, x2) tuples
  QMultiHash<int, Segment> verticals; // map x to (conid, y1, y2) tuples
};

#endif
