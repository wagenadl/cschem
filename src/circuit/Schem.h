// Schem.h

#ifndef SCHEM_H

#define SCHEM_H

#include <QSharedData>
#include <QXmlStreamReader>
#include "Circuit.h"

class Schem {
public:
  explicit Schem(bool valid=true);
  Schem(Schem const &);
  Schem(QXmlStreamReader &src);
  Schem &operator=(Schem const &);
  ~Schem();
public:
  bool isEmpty() const;
  bool isValid() const;
  Circuit const &circuit() const;
  Circuit &circuit();
  class SymbolLibrary const &library() const;
  class SymbolLibrary &library();
  class Symbol const &symbolForElement(int eltid) const;
  class Symbol const &symbolForNamedElement(QString name) const;
public:
  void saveSymbolLibrary(QXmlStreamWriter &dst,
                         bool onlySaveUsedSymbols=true) const;
private:
  QSharedDataPointer<class SchemData> d;
};

QXmlStreamWriter &operator<<(QXmlStreamWriter &, Schem const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Schem &);


#endif
