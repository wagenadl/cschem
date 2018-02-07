// PackageWidget.cpp

#include "PackageWidget.h"
#include "svg/PackageLibrary.h"
#include "svg/PackageDrawing.h"
#include "PackageBackground.h"

#include <QPainter>
#include <QDebug>
#include <QMouseEvent>
#include <math.h>

class PackageWidgetData {
public:
  PackageWidgetData() {
    lib = 0;
    scale = 0.3;
    grid = 100;
    major = 5;
    mousein = false;
  }
  PackageDrawing const &drawing();
public:
  PackageLibrary const *lib;
  QString name;
  double scale;
  double grid;
  int major;
  bool mousein;
};

class PWGeom {
public:
  PWGeom(PackageDrawing const &drw, double grid) {
    if (drw.isValid()) {
      QPicture pic = drw.picture();
      bb = pic.boundingRect();
      p0 = drw.pins()[drw.topLeftPin()].position;
      QPoint delta = p0 - bb.topLeft();
      nx = ceil(delta.x() / grid);
      ny = ceil(delta.y() / grid);
      delta = bb.bottomRight() - p0;
      mx = ceil(delta.x() / grid);
      my = ceil(delta.y() / grid);
    } else {
      nx = ny = mx = my = 0;
    }
    if (nx<2)
      nx = 2;
    if (ny<1)
      ny = 1;
    if (mx<3)
      mx = 3;
    if (my<1)
      my = 1;
  }
public:
  QPoint p0; // position of topleft pin
  int nx, ny, mx, my; // number of grid points left/above / right/below
  QRect bb; // bounding box of pixture
};

PackageWidget::PackageWidget(QWidget *parent):
  QWidget(parent), d(new PackageWidgetData) {
}

PackageWidget::~PackageWidget() {
  delete d;
}

void PackageWidget::setScale(double s) {
  d->scale = s;
  updateGeometry();
  update();
}

double PackageWidget::scale() const {
  return d->scale;
}

void PackageWidget::setGrid(double mil, int maj) {
  d->grid = mil;
  d->major = maj;
  updateGeometry();
  update();
}

double PackageWidget::gridSpacing() const {
  return d->grid;
}

int PackageWidget::gridInterval() const {
  return d->major;
}

void PackageWidget::setLibrary(class PackageLibrary const *lib) {
  d->lib = lib;
  update();
}
  
void PackageWidget::setPackage(QString name) {
  d->name = name;
  updateGeometry();
  update();
}

PackageDrawing const &PackageWidgetData::drawing() {
  if (lib)
    return lib->drawing(name);
  else
    return PackageLibrary::defaultPackages().drawing(name);
}

QSize PackageWidget::sizeHint() const {
  PackageDrawing drw = d->drawing();
  PWGeom pwg(drw, gridSpacing());
  double scl = d->scale * gridSpacing();
  return (scl*QSizeF(pwg.nx + pwg.mx, pwg.ny + pwg.my)).toSize();
}

QSize PackageWidget::minimumSizeHint() const {
  return sizeHint();
}

void PackageWidget::enterEvent(QEvent *e) {
  QWidget::enterEvent(e);
  d->mousein = true;
  update();
}

void PackageWidget::leaveEvent(QEvent *e) {
  QWidget::leaveEvent(e);
  d->mousein = false;
  update();
}

void PackageWidget::paintEvent(QPaintEvent *) {
  QPainter ptr(this);

  PackageDrawing const &drw = d->drawing();
  PWGeom pwg(drw, gridSpacing());

  // draw background
  ptr.fillRect(QRect(QPoint(0,0), size()), QColor(255,255,255));

  // draw minor grid
  ptr.setPen(QPen(QColor(220, 220, 220), 1));
  int h = height();
  int w = width();
  double gridsp = d->grid * d->scale;
  for (double x=0; x<w; x+=gridsp)
    ptr.drawLine(x, 0, x, h);
  for (double y=0; y<h; y+=gridsp)
    ptr.drawLine(0, y, w, y);

  // draw major grid
  ptr.setPen(QPen(QColor(192, 192, 192), 2));
  int x0 = pwg.nx;
  while (x0>0)
    x0 -= d->major;
  x0 += d->major;
  for (double x=x0*gridsp; x<w; x+=d->major*gridsp)
    ptr.drawLine(x, 0, x, h);
  int y0 = pwg.ny;
  while (y0>0)
    y0 -= d->major;
  y0 += d->major;
  for (double y=y0*gridsp; y<h; y+=d->major*gridsp)
    ptr.drawLine(0, y, w, y);

  if (drw.isValid()) {
    QPicture pic = drw.picture();
    ptr.translate(pwg.nx*gridsp, pwg.ny*gridsp); // target pin position
    ptr.scale(d->scale, d->scale);
    ptr.translate(-pwg.p0.x(), -pwg.p0.y());
    if (d->mousein) 
      ptr.fillRect(pwg.bb.adjusted(-50, -50, 50, 50), QColor(0, 128, 255, 64));
    ptr.drawPicture(QPoint(), pic);
  }
}

void PackageWidget::mousePressEvent(QMouseEvent *e) {
  e->accept();
  emit pressed(d->name);
}
