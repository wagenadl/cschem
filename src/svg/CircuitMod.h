// CircuitMod.h

#ifndef CIRCUITMOD_H

#define CIRCUITMOD_H

#include <QSet>
#include <QPoint>

class CircuitMod {
public:
  CircuitMod(class Circuit const &, class PartLibrary const *);
  CircuitMod(CircuitMod const &) = delete;
  CircuitMod &operator=(CircuitMod const &) = delete;
  ~CircuitMod();
  bool deleteElement(int eltid);
  bool deleteConnection(int conid);
  bool deleteConnectionSegment(int conid, int seg);
  bool removePointlessJunction(int eltid);
  bool removeOverlappingJunctions(int eltid);
  bool removeConnectionsEquivalentTo(int conid);
  bool adjustOverlappingConnections(int conid);
  bool simplifyConnection(int conid);
  bool removeIfCircular(int conid);
  bool removeIfDangling(int conid);
  bool removeAllDanglingOrCircular();
  bool translateElement(int eltid, QPoint dd);
  bool translateConnection(int conid, QPoint dd);
  bool reroute(int conid, class Circuit const &origcirc);
public:
  QSet<int> affectedConnections() const; // new, modified, or deleted
  QSet<int> affectedElements() const; // new, modified, or deleted
  Circuit const &circuit() const;
private:
  class CircuitModData *d;
};

#endif
