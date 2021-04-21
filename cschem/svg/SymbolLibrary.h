// SymbolLibrary.h

#ifndef SYMBOLLIBRARY_H

#define SYMBOLLIBRARY_H

#include "Symbol.h"
#include <QMap>

class SymbolLibrary {
public:
  SymbolLibrary(QString fn);
  SymbolLibrary();
  void merge(QString fn);
  void merge(QXmlStreamReader &sr); // must point to <svg> element
  void insert(Symbol const &);
  ~SymbolLibrary();
  QStringList symbolNames() const;
  bool contains(QString) const;
  Symbol const &symbol(QString) const;
  int scale() const;
  QPoint downscale(QPointF) const;
  QPointF upscale(QPoint) const;
  QRect downscale(QRectF) const;
  QRectF upscale(QRect) const;
  QPointF nearestGrid(QPointF) const;
  double lineWidth() const;
  QPolygonF simplifyPath(QPolygonF) const;
  static SymbolLibrary const &defaultSymbols();
private:
  void scanSymbols(XmlElement const &src);
private:
  QMap<QString, Symbol> symbols_;
};

#endif
