// CleanupConnection.cpp

#include "CleanupConnection.h"
#include "file/Circuit.h"
#include "Scene.h"


class CCData {
public:
  CCData(Scene *scene): scene(scene) {
    circ = scene->circuit();
  }
  void removeImmediateRedundancy(int con);
  void perhapsRewirePointlessJunction(int elt);
  void removeJunctionWithCrossRewire(int elt);
  void removeDanglingConnections();
  void rewireOverlapping(int con);
public:
  Scene *scene;
  QSet<int> acons;
  QSet<int> ajuncs;
  Circuit circ;
};

void CCData::removeImmediateRedundancy(int c) {
  /* Finds out whether there is any other connection in the circuit
     with endpoints identical to us, and if so, removes it.
  */
  Connection con = circ.connection(c);
  QSet<int> equivs;
  for (auto const &con1: circ.connections()) 
    if (con1.id() != c && con.isEquivalentTo(con1))
      equivs << con1.id();
}

CleanupConnection::CleanupConnection(Scene *scene): d(new CCData(scene)) {
}

CleanupConnection::~CleanupConnection() {
  delete d;
}

void CleanupConnection::cleanup(int con) {
  //
}

QSet<int> CleanupConnection::affectedConnections() {
  return d->acons;
}

QSet<int> CleanupConnection::affectedJunctions() {
  return d->ajuncs;
}

Circuit const &CleanupConnection::updatedCircuit() {
  return d->circ;
}

