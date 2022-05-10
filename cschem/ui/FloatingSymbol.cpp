// FloatingSymbol.cpp

#include "FloatingSymbol.h"
#include "svg/Symbol.h"

class FloatingSymbolData {
public:
  FloatingSymbolData(Symbol const &sym1): sym(sym1) {
  bbOrigin = sym.bbOrigin();
  sCM = sym.shiftedBBox().center();
  name = sym.name();
  }
public:
  Symbol sym;
  QPointF bbOrigin;
  QPointF sCM;
  QString name;
};

FloatingSymbol::FloatingSymbol(Symbol const &symbol):
  d(new FloatingSymbolData(symbol)) {
  setRenderer(d->sym.renderer());
}
  
FloatingSymbol::~FloatingSymbol() {
  delete d;
}

Symbol const &FloatingSymbol::symbol() const {
  return d->sym;
}
      

void FloatingSymbol::setSymbolPosition(QPointF p) {
  setPos(p - d->bbOrigin);
}

QPointF FloatingSymbol::symbolPosition() const {
  return pos() + d->bbOrigin;
}

QPointF FloatingSymbol::shiftedCenter() const {
  return d->sCM;
}

QString FloatingSymbol::name() const {
  return d->name;
}

