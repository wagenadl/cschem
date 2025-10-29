// SymbolLibrary.cpp

#include "SymbolLibrary.h"
#include <QDebug>
#include <QSvgRenderer>
#include <math.h>
#include <QFile>
#include <QPolygonF>

SymbolLibrary::SymbolLibrary() {
}

SymbolLibrary::SymbolLibrary(QString fn) {
  merge(fn);
}


void SymbolLibrary::merge(QString fn) {
  QFile file(fn);
  if (!file.open(QFile::ReadOnly)) {
    qDebug() << "Failed to open symbol library";
    return;
  }

  QXmlStreamReader sr(&file);
  while (!sr.atEnd()) {
    sr.readNext();
    if (sr.isStartElement() && sr.name()==QStringLiteral("svg")) 
      merge(sr);
  }
}

void SymbolLibrary::merge(QXmlStreamReader &sr) {
  int nOld = symbolNames().size();

  XmlElement svg(sr);
  if (svg.isValid()) {
    scanSymbols(svg);
  } else {
    qDebug() << "Failed to parse svg";
    return;
  }
  if (symbolNames().size() == nOld)
    qDebug() << "(No symbols loaded)";
}

SymbolLibrary::~SymbolLibrary() {
}

void SymbolLibrary::insert(Symbol const &p) {
  symbols_[p.typeName()] = p;
}  

void SymbolLibrary::scanSymbols(XmlElement const &src) {
  if (src.name()=="g") {
    QString label = src.label();
    if (label.startsWith("part:") || label.startsWith("port:")
        || label=="junction") {
      insert(Symbol(src));
      return;
    }
  }
  
  for (auto &c: src.children())
    if (c.type()==XmlNode::Type::Element)
      scanSymbols(c.element());
}

QStringList SymbolLibrary::symbolNames() const {
  return symbols_.keys();
}

bool SymbolLibrary::contains(QString name) const {
  return symbols_.contains(name);
}

Symbol const &SymbolLibrary::symbol(QString name) const {
  int idx = name.indexOf("::");
  if (idx>=0)
    name = name.left(idx); // strip popup name
  static Symbol nil;
  auto it = symbols_.find(name);
  if (it == symbols_.end())
    return nil;
  else
    return *it;
}

int SymbolLibrary::scale() const {
  return 7; // infer this from symbol drawings?
}

QPoint SymbolLibrary::downscale(QPointF p) const {
  return (p/scale()).toPoint();
}

QPointF SymbolLibrary::upscale(QPoint p) const {
  return QPointF(p*scale());
}

QPointF SymbolLibrary::nearestGrid(QPointF p) const {
  return upscale(downscale(p));
}

QRect SymbolLibrary::downscale(QRectF r) const {
  return QRect(downscale(r.topLeft()), downscale(r.bottomRight()));
}

QRectF SymbolLibrary::upscale(QRect r) const {
  return QRectF(upscale(r.topLeft()), upscale(r.bottomRight()));
}

double SymbolLibrary::lineWidth() const {
  return 1.5;
}

QPolygonF SymbolLibrary::simplifyPath(QPolygonF pp) const {
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

SymbolLibrary const &SymbolLibrary::defaultSymbols() {
  static SymbolLibrary lib(":symbols.svg");
  return lib;
}
