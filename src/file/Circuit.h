// Circuit.h

#ifndef CIRCUIT_H

#define CIRCUIT_H

#include <QSharedData>
#include <QXmlStreamReader>
#include "Element.h"
#include "Connection.h"
#include <QMap>

class Circuit {
public:
  Circuit();
  Circuit(Circuit const &);
  Circuit(QXmlStreamReader &src);
  Circuit &operator=(Circuit const &);
  ~Circuit();
public:
  QMap<int, class Element> const &elements() const;
  QMap<int, class Element> &elements();
  QMap<int, class Connection> const &connections() const;
  QMap<int, class Connection> &connections();
private:
  QSharedDataPointer<class CircuitData> d;
};

QXmlStreamWriter &operator<<(QXmlStreamWriter &, Circuit const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Circuit &);

#endif
