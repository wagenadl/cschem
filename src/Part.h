// Part.h

#ifndef PART_H

#define PART_H

#include "XmlElement.h"
#include <QRect>
#include <QSharedDataPointer>

class Part {
public:
  Part(XmlElement const &elt);
  ~Part();
  Part(Part const &);
  Part &operator=(Part const &);
  XmlElement const &element() const;
  QString name() const;
  QMap<QString, QPoint> const &pins() const;
  QPoint pinPosition(QString pinname) const;
  bool isValid() const;
  void setBBox(QRect);
  QRect bbox() const;
private:
  void scanPins(XmlElement const &elt);
private:
  QSharedDataPointer<class PartData> d;
};

#endif
