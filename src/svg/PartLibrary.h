// PartLibrary.h

#ifndef PARTLIBRARY_H

#define PARTLIBRARY_H

#include "Part.h"
#include <QSharedPointer>

class PartLibrary {
public:
  PartLibrary(QString fn);
  PartLibrary();
  void merge(QString fn);
  void merge(QXmlStreamReader &sr);
  void insert(Part const &);
  ~PartLibrary();
  QStringList partNames() const;
  bool contains(QString) const;
  Part part(QString) const;
  QByteArray partSvg(QString name) const;
  QSharedPointer<class QSvgRenderer> renderer(QString name) const;
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
  QList<Part> partslist_;
  QMap<QString, Part *> parts_;
  mutable QMap<QString, QSharedPointer<QSvgRenderer> > renderers_;
};

#endif
