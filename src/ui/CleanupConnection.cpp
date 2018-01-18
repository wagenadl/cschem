// CleanupConnection.cpp

#include "CleanupConnection.h"
#include "file/Circuit.h"
#include "Scene.h"


class CCData {
public:
  CCData(Scene *scene): scene(scene) {
    circ = scene->circuit();
  }
  void deleteConnection(int con);
  void deleteElement(int elt);
  void removeImmediateRedundancy(int con);
  void rewirePointlessJunction(int elt);
  void deleteJunctionWithCrossRewire(int elt);
  void deleteDanglingConnections();
  void rewireOverlapping(int con);
public:
  Scene *scene;
  QSet<int> acons;
  QSet<int> aelts;
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

QSet<int> CleanupConnection::affectedElements() {
  return d->aelts;
}

Circuit const &CleanupConnection::updatedCircuit() {
  return d->circ;
}

void CleanupConnection::deleteConnection(int con) {
  d->deleteConnection(con);
}

void CleanupConnection::deleteElement(int elt) {
  d->deleteElement(elt);
}
