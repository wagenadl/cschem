// Part.h

#ifndef PART_H

#define PART_H

#include "XmlElement.h"
#include <QRectF>

class Part {
public:
  Part(XmlElement const *elt);
  XmlElement const *element() const { return elt_; }
  QString name() const { return name_; }
  QList<QPoint> pinPositions() const { return pinPositions_; }
  QStringList pinNames() const { return pinNames_; }
  QPoint pinPosition(QString pinname) const;
  bool isValid() const { return valid_; }
  void setBBox(QRectF);
  QRectF bbox() const { return bbox_; }
private:
  void scanPins(XmlElement const *elt);
private:
  XmlElement const *elt_;
  QString name_;
  QList<QPoint> pinPositions_;
  QStringList pinNames_;
  bool valid_;
  QRectF bbox_;
};

#endif
