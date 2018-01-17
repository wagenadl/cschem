// Schem.h

#ifndef SCHEM_H

#define SCHEM_H

#include <QSharedData>
#include <QXmlStreamReader>
#include "Circuit.h"
//#include "Symbols.h"

class Schem {
public:
  explicit Schem(bool valid=true);
  Schem(Schem const &);
  Schem(QXmlStreamReader &src);
  Schem &operator=(Schem const &);
  ~Schem();
public:
  Circuit const &circuit() const;
  void setCircuit(Circuit const &);
  //Symbols const &symbols() const;
  //void setSymbols(Symbols const &);
  bool isEmpty() const;
  bool isValid() const;
  class SymbolLibrary const &library() const;
  void selectivelyUpdateLibrary(SymbolLibrary const &);
public:
  void saveSvg(QXmlStreamWriter &dst) const;
private:
  QSharedDataPointer<class SchemData> d;
};

QXmlStreamWriter &operator<<(QXmlStreamWriter &, Schem const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Schem &);


#endif
