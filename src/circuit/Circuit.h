// Circuit.h

#ifndef CIRCUIT_H

#define CIRCUIT_H

#include <QSharedData>
#include <QXmlStreamReader>
#include "Element.h"
#include "Connection.h"
#include <QMap>
#include <QPoint>

class Circuit {
public:
  Circuit();
  Circuit(Circuit const &);
  Circuit &operator=(Circuit const &);
  ~Circuit();
  explicit Circuit(Connection const &); // create a circuit consisting
  // of just one connection
public:
  void insert(Element const &); // or replace
  void insert(Connection const &);
  void removeElement(int id);
  /* Removes an element. Connections to/from the element are also deleted. */
  void removeConnection(int id);
  /* Removes a connection. */
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
  void translate(QPoint dleta);
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
  Circuit &operator+=(Circuit const &);
  /* Bluntly merges two circuits, not worrying about overlapping
     connections, conflicting IDs, or anything like that. */
  int availableNumber(QString pfx) const; 
  /* Returns a number that is not yet in use in the circuit for a name starting
     with PFX. */
 QString autoName(QString sym) const;
  /* Creates an automatic name for an element with symbol SYM. */
public:
  QMap<int, class Element> const &elements() const;
  QMap<int, class Connection> const &connections() const;
  Element const &element(int) const;
  Connection const &connection(int) const;
  bool isEmpty() const;
  bool isValid() const;
private:
  QSharedDataPointer<class CircuitData> d;
  friend QXmlStreamWriter &operator<<(QXmlStreamWriter &, Circuit const &);
  friend QXmlStreamReader &operator>>(QXmlStreamReader &, Circuit &);
};

QXmlStreamWriter &operator<<(QXmlStreamWriter &, Circuit const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Circuit &);

#endif
