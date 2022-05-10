// FloatingSymbol.h

#ifndef FLOATINGSYMBOL_H

#define FLOATINGSYMBOL_H

#include "ui/SvgItem.h"

class FloatingSymbol: public SvgItem {
public:
  FloatingSymbol(class Symbol const &symbol);
  ~FloatingSymbol();
  FloatingSymbol(FloatingSymbol const &) = delete;
  FloatingSymbol &operator=(FloatingSymbol const &) = delete;
  void setSymbolPosition(QPointF); // set position of first pin
  Symbol const &symbol() const;
  QPointF symbolPosition() const;
  QPointF shiftedCenter() const; // position of center relative to first pin
  QString name() const;
private:
  class FloatingSymbolData *d;
};

#endif
