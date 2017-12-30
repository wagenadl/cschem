// Part.h

#ifndef PART_H

#define PART_H

#include "XmlElement.h"
#include <QRectF>
#include <QSharedDataPointer>

class Part {
public:
  Part(XmlElement const &elt);
  Part();
  ~Part();
  Part(Part const &);
  Part &operator=(Part const &);
  XmlElement const &element() const;
  QString name() const;
  QPointF pinPosition(QString pinname) const; // relative to bbox
  QStringList pinNames() const; // sorted
  QPointF origin() const; // position of first pin relative to bbox
  bool isValid() const;
  void setBBox(QRectF);
  QRectF bbox() const; // in original svg
  QRectF shiftedBBox() const; // bbox as if first pin were at (0,0)
  QString contentsSvgId() const;
  QString pinSvgId(QString pinname) const;
  void setAbsPinPosition(QString pinname, QPointF pos);
private:
  void scanPins(XmlElement const &elt);
private:
  QSharedDataPointer<class PartData> d;
};

#endif
