// Schem.h

#ifndef SCHEM_H

#define SCHEM_H

#include <QSharedData>
#include <QXmlStreamReader>
#include "Circuit.h"
#include "Parts.h"

class Schem {
public:
  Schem();
  Schem(Schem const &);
  Schem(QXmlStreamReader &src);
  Schem &operator=(Schem const &);
  ~Schem();
public:
  Circuit const &circuit() const;
  Circuit &circuit();
  Parts const &parts() const;
  Parts &parts();
private:
  QSharedDataPointer<class SchemData> d;
};

QXmlStreamWriter &operator<<(QXmlStreamWriter &, Schem const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Schem &);


#endif
