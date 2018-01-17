// FloatingSymbol.h

#ifndef FLOATINGSYMBOL_H

#define FLOATINGSYMBOL_H

#include "qt/SvgItem.h"

class FloatingSymbol: public SvgItem {
public:
  FloatingSymbol(class Symbol const &symbol);
  ~FloatingSymbol();
  FloatingSymbol(FloatingSymbol const &) = delete;
  FloatingSymbol &operator=(FloatingSymbol const &) = delete;
  void setSymbolPosition(QPointF); // set position of first pin
  QPointF symbolPosition() const;
  QPointF shiftedCenter() const; // position of center relative to first pin
  QString name() const;
private:
  class FloatingSymbolData *d;
};

#endif
