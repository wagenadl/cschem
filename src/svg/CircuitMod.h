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
  bool rotateElement(int eltid, int steps=1);
  /* Rotates the element by STEPS x 90 degrees ccw. True if successful. */
  bool deleteElement(int eltid);
  /* Leaves connected lines dangling. Exception: if the deleted
     element is a junction with four connections, through-connections
     are preserved, and if it is a junction with two connections, those
     are reconnected.  Deletes zero-length dangling lines. True if
     successful. */
  bool deleteElements(QSet<int> eltids);
  /* Also cuts all internal connections and external connections that started
     out dangling. Other external connections are made dangling. True if
     successful. */
  bool deleteConnection(int conid);
  /* Calls removePointlessJunction to remove and rewire adjacent
     junctions that are left with few than three connections. True if
     successful.
   */
  bool deleteConnectionSegment(int conid, int seg);
  /* Removes one segment from a connection splitting the connection
     into two dangling parts. Zero length stubs are removed and
     removePointlessJunction is called to cleanup adjacent
     junctions. True if successful. */
  bool removePointlessJunction(int eltid);
  /* If eltid refers to a junction with two or fewer connections,
     removes that junction from the circuit and rewires its
     connections. True if successful. */
  bool removeOverlappingJunctions(int eltid);
  /* Checks the circuit for junctions that lie at the same location
     as the referenced junction. Removes those junctions are rewires
     their connections to the referenced junction. True if
     successful. NB: The current implementation only investigates
     junctions that are directly connected to the referenced junction,
     so complex loops go undetected. */
  bool removeConnectionsEquivalentTo(int conid);
  /* Checks the circuit for connections that begin at the same pin as
     the referenced connection and end at the same pin as the
     referenced connection, or vice versa. Such connections are
     removed. True if successful. NB: This does not check for complex
     loops. Dangling connections are only equivalent if their geometries
     match. */
  bool adjustOverlappingConnections(int conid);
  /* Checks the circuit for connections that share endpoints with the
     referenced connection. If the initial segments of such
     connections overlap in space, removes the overlap, creating a new
     junction at the end of the overlap (or moving junctions if
     appropriate). True if successful.
   */
  bool simplifyConnection(int conid);
  /* Combines adjacent segments that go in the same direction. If the
     connection is dangling and ends up with zero length, it is
     removed from the circuit. True if successful. */
  bool removeIfInvalid(int conid);
  /* Removes the connection if it is invalid. True if succesful. */
  bool removeIfDangling(int conid);
  /* Removes the connection if it is dangling. True if succesful. */
  bool removeAllDanglingOrInvalid();
  /* Removes all dangling or invalid connections from the
     circuit. True if successful. */
  bool translateElement(int eltid, QPoint dd);
  /* Simply translates the named element by the given amount. Does not check
     for resulting overlaps. True if successful. */
  bool translateConnection(int conid, QPoint dd);
  /* Simply translates the named connection by the given
     amount. Endpoints attached to pins are not moved. Does not check
     for resulting overlaps. True if succesful. */
  int injectJunction(int conid, QPoint at);
  /* Splits the given connection into two parts and places a junction
     between them. Returns element ID of created junction. If called
     on an invalid connection, returns -1 and does not create a
     junction. If the given point does not lie on (or very near) the
     connection, also returns -1 and does not create a junction.  This
     call creates a "pointless" junction, so be sure to remove it
     later. */
  bool reroute(int conid, class Circuit const &origcirc);
public:
  QSet<int> affectedConnections() const; // new, modified, or deleted
  QSet<int> affectedElements() const; // new, modified, or deleted
  Circuit const &circuit() const;
private:
  class CircuitModData *d;
};

#endif
