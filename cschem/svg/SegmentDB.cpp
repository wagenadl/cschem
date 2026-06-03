// SegmentDB.cpp

#include "SegmentDB.h"

SegmentDB::Segment::Segment(int conid, int a, int b): conid(conid) {
  if (a > b) {
    start = b; end = a;
  } else {
    start = a; end = b;
  }
}

SegmentDB::SegmentDB(): _any(false) {
}

void SegmentDB::insert(int conid, QPoint p1, QPoint p2) {
  if (p1.x() == p2.x()) 
    verticals.insert(p1.x(), Segment(conid, p1.y(), p2.y()));
  else if (p1.y() == p2.y())
    horizontals.insert(p1.y(), Segment(conid, p1.x(), p2.x()));
  // ignore non-orthogonal elements
  _any = true;
}

int SegmentDB::find(QPoint p) const {
  auto its = verticals.equal_range(p.x());
  for (auto it = its.first; it != its.second; ++it) 
    if (p.y() >= it.value().start && p.y() <= it.value().end)
      return it.value().conid;
  its = horizontals.equal_range(p.y());
  for (auto it = its.first; it != its.second; ++it) 
    if (p.x() >= it.value().start && p.x() <= it.value().end)
      return it.value().conid;
  return -1;
}

SegmentDB SegmentDB::restricted(QSet<int> cons,
                                bool inc_not_exc) const {
  SegmentDB db;
  db._any = true;
  for (auto it = horizontals.begin(); it != horizontals.end(); it++) 
    if (cons.contains(it.value().conid) == inc_not_exc)
      db.horizontals.insert(it.key(), it.value());
  for (auto it = verticals.begin(); it != verticals.end(); it++) 
    if (cons.contains(it.value().conid) == inc_not_exc)
      db.verticals.insert(it.key(), it.value());
  return db;
}
