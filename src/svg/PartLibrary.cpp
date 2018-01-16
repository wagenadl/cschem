// PartLibrary.cpp

#include "PartLibrary.h"
#include <QDebug>
#include <QSvgRenderer>
#include <math.h>

PartLibrary::PartLibrary() {
}

PartLibrary::PartLibrary(QString fn) {
  merge(fn);
}


void PartLibrary::merge(QString fn) {
  QFile file(fn);
  if (!file.open(QFile::ReadOnly)) {
    qDebug() << "Failed to open part library";
    return;
  }

  QXmlStreamReader sr(&file);
  while (!sr.atEnd()) {
    sr.readNext();
    if (sr.isStartElement() && sr.qualifiedName()=="svg") 
      merge(sr);
  }
}

void PartLibrary::merge(QXmlStreamReader &sr) {
  int nOld = partNames().size();

  XmlElement svg(sr);
  if (svg.isValid()) {
    scanParts(svg);
  } else {
    qDebug() << "Failed to parse svg";
    return;
  }
  if (partNames().size() == nOld)
    qDebug() << "No parts loaded";
}

PartLibrary::~PartLibrary() {
}

void PartLibrary::insert(Part const &p) {
  parts_[p.name()] = p;
  qDebug() << "Inserted part" << p.name() << "with pins" << p.pinNames();
}  

void PartLibrary::scanParts(XmlElement const &src) {
  if (src.qualifiedName()=="g") {
    QString label = src.attributes().value("inkscape:label").toString();
    if (label.startsWith("part:") || label.startsWith("port:")
        || label=="junction") {
      insert(Part(src));
      return;
    }
  }
  
  for (auto &c: src.children())
    if (c.type()==XmlNode::Type::Element)
      scanParts(c.element());
}

QStringList PartLibrary::partNames() const {
  return parts_.keys();
}

bool PartLibrary::contains(QString name) const {
  return parts_.contains(name);
}

Part PartLibrary::part(QString name) const {
  if (parts_.contains(name))
    return parts_[name];
  else
    return Part();
}

int PartLibrary::scale() const {
  return 7; // infer this from part drawings?
}

QPoint PartLibrary::downscale(QPointF p) const {
  return (p/scale()).toPoint();
}

QPointF PartLibrary::upscale(QPoint p) const {
  return QPointF(p*scale());
}

QPointF PartLibrary::nearestGrid(QPointF p) const {
  return upscale(downscale(p));
}

QRect PartLibrary::downscale(QRectF r) const {
  return QRect(downscale(r.topLeft()), downscale(r.bottomRight()));
}

QRectF PartLibrary::upscale(QRect r) const {
  return QRectF(upscale(r.topLeft()), upscale(r.bottomRight()));
}

double PartLibrary::lineWidth() const {
  return 1.5;
}

QPolygonF PartLibrary::simplifyPath(QPolygonF pp) const {
  for (auto &p: pp)
    p = nearestGrid(p);
  
  int n = 1;
  while (n<pp.size() - 1) {
    if ((pp[n-1].x()==pp[n].x() && pp[n+1].x()==pp[n].x())
        || (pp[n-1].y()==pp[n].y() && pp[n+1].y()==pp[n].y()))
      pp.removeAt(n);
    else
      n++;
  }
  return pp;
}

PartLibrary const &PartLibrary::defaultLibrary() {
  static PartLibrary lib(":symbols.svg");
  return lib;
}
