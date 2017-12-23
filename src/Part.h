// Part.h

#ifndef PART_H

#define PART_H

#include "XmlElement.h"
#include <QRect>

class Part {
public:
  Part(XmlElement const *elt);
  XmlElement const *element() const { return elt_; }
  QString name() const { return name_; }
  QMap<QString, QPoint> pins() const { return pins_; }
  QPoint pinPosition(QString pinname) const;
  bool isValid() const { return valid_; }
  void setBBox(QRect);
  QRect bbox() const { return bbox_; }
private:
  void scanPins(XmlElement const *elt);
private:
  XmlElement const *elt_;
  QString name_;
  QMap<QString, QPoint> pins_;
  bool valid_;
  QRect bbox_;
};

#endif
