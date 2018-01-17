// FloatingSymbol.cpp

#include "FloatingSymbol.h"
#include "svg/Symbol.h"

class FloatingSymbolData {
public:
  FloatingSymbolData() {
  }
public:
  QPointF bbOrigin;
  QPointF sCM;
  QString name;
};

FloatingSymbol::FloatingSymbol(Symbol const &symbol):
  d(new FloatingSymbolData()) {
  d->bbOrigin = symbol.bbOrigin();
  d->sCM = symbol.shiftedBBox().center();
  d->name = symbol.name();
  setRenderer(symbol.renderer());
}
  
FloatingSymbol::~FloatingSymbol() {
  delete d;
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

