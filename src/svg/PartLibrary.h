// PartLibrary.h

#ifndef PARTLIBRARY_H

#define PARTLIBRARY_H

#include "Part.h"

class PartLibrary {
public:
  PartLibrary(QString fn);
  PartLibrary();
  void merge(QString fn);
  void merge(QXmlStreamReader &sr); // must point to <svg> element
  void insert(Part const &);
  ~PartLibrary();
  QStringList partNames() const;
  bool contains(QString) const;
  Part part(QString) const;
  int scale() const;
  QPoint downscale(QPointF) const;
  QPointF upscale(QPoint) const;
  QRect downscale(QRectF) const;
  QRectF upscale(QRect) const;
  QPointF nearestGrid(QPointF) const;
  double lineWidth() const;
  QPolygonF simplifyPath(QPolygonF) const;
  static PartLibrary const &defaultLibrary();
private:
  void scanParts(XmlElement const &src);
private:
  QMap<QString, Part> parts_;
};

#endif
