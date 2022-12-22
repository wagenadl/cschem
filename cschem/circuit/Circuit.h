// Circuit.h

#ifndef CIRCUIT_H

#define CIRCUIT_H

#include <QXmlStreamReader>
#include "Element.h"
#include "Connection.h"
#include "Textual.h"
#include <QMap>
#include <QPoint>
#include "SafeMap.h"
#include "PinID.h"

class Circuit {
public:
  Circuit();
  explicit Circuit(Connection const &);
  /* create a circuit consisting of just one connection */
public:
  void insert(Element const &);
  void insert(Connection const &);
  void insert(Textual const &);
  void removeElementWithConnections(int id);
  /* Removes an element. Connections to/from the element are also deleted. */
  QSet<int> connectionsTo(QSet<int> ids) const;
  /* Connections with "toId" in the set ("fromId" is irrelevant) */
  QSet<int> connectionsFrom(QSet<int> ids) const;
  /* Connections with "fromId" in the set ("toId" is irrelevant) */
  QSet<int> connectionsIn(QSet<int> ids) const;
  /* Connections with both "toId" and "fromId" in the set, or with one
     in the set and the other dangling. */
  QSet<int> connectionsOn(int id) const;
  /* Connections that either start or end at any pin of given element */
  QSet<int> connectionsOn(int id, QString pin) const;
  QSet<int> connectionsOn(class PinID const &) const;
  /* Connections that either start or end at given pin of given element */
  void translate(QSet<int> eltids, QPoint delta);
  /* Does not take care of edge connections */
  void translate(QPoint delta);
  /* Translates entire circuit */
  int maxId() const;
  /* Returns the largest ID number in use in this circuit. */
  int renumber(int start=1, QMap<int, int> *mapout=0);
  /* Renumbers elements and connections to start at the given ID value.
     Returns the largest value assigned. Connections that refer to non-
     existent elements are made dangling or removed entirely if they
     become invalid. */
  Circuit subset(QSet<int> elts) const;
  /* Creates a copy of a subset of this circuit containing the
     indicated elements, all connections between those elements, and
     all connections onto those elements that have a dangling other
     end.  Note that the created subset may have "pointless" junctions
     (See CircuitMod).
   */
  Circuit restset(QSet<int> elts) const;
  /* Creates a copy of what is left of this circuit when the subset is
     taken out. */
  void merge(Circuit const &);
  /* Bluntly merges two circuits, not worrying about overlapping
     connections, conflicting IDs, or anything like that. */
  int availableNumber(QString pfx) const; 
  /* Returns a number that is not yet in use in the circuit for a name starting
     with PFX. */
  QString autoName(QString sym) const;
  /* Creates an automatic name for an element with symbol SYM. */
  bool isEmpty() const;
  bool isValid() const;
  void invalidate();
  int elementByName(QString) const;
  /* Returns ID for named element or -1. Prefers to return container. */
  QSet<int> containedElements(int containerId) const;
  int containerOf(int elt) const; // or -1
  QSet<QString> allNames() const;
  void verifyIDs() const;
  QString humanPinName(PinID pin) const;
  void newUUID();
public:
  SafeMap<int, Element> elements;
  SafeMap<int, Connection> connections;
  SafeMap<int, Textual> textuals;
  QString uuid;
private:
  bool valid;
};

QXmlStreamWriter &operator<<(QXmlStreamWriter &, Circuit const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Circuit &);
QDebug &operator<<(QDebug &, Circuit const &);

#endif
