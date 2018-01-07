// CircuitModData.h

#ifndef CIRCUITMODDATA_H

#define CIRCUITMODDATA_H

#include "CircuitMod.h"
#include "Geometry.h"
#include "file/Circuit.h"
#include "file/PinID.h"
#include "svg/Router.h"
#include "svg/Part.h"
#include "svg/PartLibrary.h"
#include "file/Net.h"
#include <QDebug>


struct OverlapResult {
  OverlapResult():
    overlap(false), allOfFirstSegmentA(false), allOfFirstSegmentB(false) { }
  operator bool() const { return overlap; }
  bool overlap;
  bool allOfFirstSegmentA;
  bool allOfFirstSegmentB;
};

class CircuitModData {
public:
  CircuitModData(Circuit const &circ, PartLibrary const *lib):
    circ(circ), lib(lib) {
  }
  OverlapResult overlappingStart(Connection const &a, Connection const &b) const;
  bool removePointlessJunction(int id);
  void removeOverlap(int ida, int idb, OverlapResult over);
  bool removeOverlappingJunctions(int id);
  void rewirePassthroughs(int id, QSet<int> cc);
  void makeDanglingAt(int id, QSet<int> cc);
  bool rewire(QSet<int> cc, PinID old, PinID new_);
  int injectJunction(int conid, QPoint at);
  int addInPlaceConnection(PinID pidbot, PinID pidtop, QPoint pos);
  /* Create a connection between two pins in the same location. May
     return the ID of an existing or newly created junction that should
     be tested for overlap and pointlessness. */
  int addInPlaceConnection(PinID pidbot, int conid, QPoint pos);
  /* Create a connection between a pin and a connection in the same
     location. Same. */
  int addInPlaceConnection(int conbot, int contop, QPoint pos);
  /* Create a connection between two connections in the same
     location. Same. */
  /* The following insert into or drop from the circuit and also mark
     the "affected" lists. They have no intelligence, unlike the functions above. */
  void insert(Connection const &);
  void insert(Element const &);
  void drop(Connection const &);
  void drop(Element const &);
  void insertOrDrop(bool cond, Connection const &);
  void insertOrDrop(bool cond, Element const &);
  void dropOrInsert(bool cond, Connection const &);
  void dropOrInsert(bool cond, Element const &);
  void dropCon(int);
  void dropElt(int);
 public:
  Circuit circ;
  PartLibrary const *lib;
  QSet<int> acons;
  QSet<int> aelts;
};

#endif
