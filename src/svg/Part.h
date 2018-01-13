// Part.h

#ifndef PART_H

#define PART_H

#include "XmlElement.h"
#include <QRectF>
#include <QSharedDataPointer>
#include <QSharedPointer>

class Part {
public:
  Part(XmlElement const &elt);
  Part();
  ~Part();
  Part(Part const &);
  Part &operator=(Part const &);
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
  static void forgetRenderer(Part const &);
  QSharedPointer<class QSvgRenderer> renderer() const;
private:
  QSharedDataPointer<class PartData> d;
};

#endif
