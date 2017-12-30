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
  Circuit(QXmlStreamReader &src);
  Circuit &operator=(Circuit const &);
  ~Circuit();
public:
  void insert(Element const &);
  void insert(Connection const &);
  void remove(int id);
  QSet<int> connectionsTo(QSet<int> ids) const;
  /* Connections with "toId" in the set ("fromId" is irrelevant) */
  QSet<int> connectionsFrom(QSet<int> ids) const;
  /* Connections with "fromId" in the set ("toId" is irrelevant) */
  QSet<int> connectionsIn(QSet<int> ids) const;
  /* Connections with both "toId" and "fromId" in the set */
  QSet<int> connectionsOn(int id, QString pin) const;
  /* Connections that either start or end at given pin of given element */
  void translate(QSet<int> ids, QPoint delta);
  /* Does not take care of edge connections */
public:
  QMap<int, class Element> const &elements() const;
  QMap<int, class Element> &elements();
  QMap<int, class Connection> const &connections() const;
  QMap<int, class Connection> &connections();
  Element const &element(int) const;
  Element &element(int);
  Connection const &connection(int) const;
  Connection &connection(int);
private:
  QSharedDataPointer<class CircuitData> d;
};

QXmlStreamWriter &operator<<(QXmlStreamWriter &, Circuit const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Circuit &);

#endif
