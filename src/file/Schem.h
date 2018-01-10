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
  void setCircuit(Circuit const &);
  Parts const &parts() const;
  void setParts(Parts const &);
  bool isEmpty() const;
  class PartLibrary const &library() const;
  void selectivelyUpdateLibrary(PartLibrary const &);
public:
  void saveSvg(QXmlStreamWriter &dst) const;
private:
  QSharedDataPointer<class SchemData> d;
};

QXmlStreamWriter &operator<<(QXmlStreamWriter &, Schem const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Schem &);


#endif
