// Symbol.h

#ifndef SYMBOL_H

#define SYMBOL_H

#include "XmlElement.h"
#include <QRectF>
#include <QSharedDataPointer>
#include <QSharedPointer>

class Symbol {
public:
  Symbol(XmlElement const &elt);
  Symbol();
  ~Symbol();
  Symbol(Symbol const &);
  Symbol &operator=(Symbol const &);
  XmlElement const &element() const;
  QString name() const;
  QPointF bbPinPosition(QString pinname) const; // relative to TL of bbox
  QStringList pinNames() const; // sorted
  QPointF bbOrigin() const; // position of first pin relative to TL of bbox
  bool isValid() const;
  QRectF svgBBox() const; // in original svg
  QPointF svgOrigin() const; // position of first pin in svg
  QPointF shiftedPinPosition(QString pinname) const;
  // as if first pin were at (0,0)
  QRectF shiftedBBox() const; // bbox as if first pin were at (0,0)
  QByteArray toSvg() const;
  void writeSvg(QXmlStreamWriter &sw) const;
  static void writeNamespaces(QXmlStreamWriter &sw);
  static void forgetRenderer(Symbol const &);
  QSharedPointer<class QSvgRenderer> renderer() const;
  QRectF shiftedAnnotationBBox(QString id) const;
  Qt::Alignment annotationAlignment(QString id) const;
private:
  QSharedDataPointer<class SymbolData> d;
};

#endif
