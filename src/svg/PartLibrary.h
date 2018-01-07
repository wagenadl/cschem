// PartLibrary.h

#ifndef PARTLIBRARY_H

#define PARTLIBRARY_H

#include "Part.h"

class PartLibrary {
public:
  PartLibrary();
  void merge(QString fn);
  void insert(Part const &);
  PartLibrary(PartLibrary const &) = delete;
  PartLibrary const &operator=(PartLibrary const &) = delete;
  ~PartLibrary();
  QStringList partNames() const;
  Part part(QString) const;
  QByteArray partSvg(QString name) const;
  class QSvgRenderer *renderer(QString name) const;
  int scale() const;
  QPoint downscale(QPointF) const;
  QPointF upscale(QPoint) const;
  QRect downscale(QRectF) const;
  QRectF upscale(QRect) const;
  QPointF nearestGrid(QPointF) const;
  double lineWidth() const;
  QPolygonF simplifyPath(QPolygonF) const;
  static PartLibrary const *defaultLibrary();
private:
  void scanParts(XmlElement const &src);
private:
  QList<Part> partslist_;
  QMap<QString, Part const *> parts_;
  mutable QMap<QString, QSvgRenderer *> renderers_;
};

#endif
