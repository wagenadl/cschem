// Circuit.h

#ifndef CIRCUIT_H

#define CIRCUIT_H

#include <QSharedData>
#include <QXmlStreamReader>
#include "Component.h"
#include "Port.h"
#include "Junction.h"
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
  QMap<int, class Component> const &components() const;
  QMap<int, class Component> &components();
  QMap<int, class Port> const &ports() const;
  QMap<int, class Port> &ports();
  QMap<int, class Junction> const &junctions() const;
  QMap<int, class Junction> &junctions();
  QMap<int, class Connection> const &connections() const;
  QMap<int, class Connection> &connections();
private:
  QSharedDataPointer<class CircuitData> d;
};

QXmlStreamWriter &operator<<(QXmlStreamWriter &, Circuit const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Circuit &);


#endif
