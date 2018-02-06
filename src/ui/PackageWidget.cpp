// PackageWidget.cpp

#include "PackageWidget.h"
#include "svg/PackageLibrary.h"
#include "svg/PackageDrawing.h"

#include <QPainter>
#include <QDebug>
#include <QMouseEvent>

class PackageWidgetData {
public:
  PackageDrawing const &drawing();
public:
  PackageLibrary const *lib;
  QString name;
  bool freescale;
  double scale;
};

PackageWidget::PackageWidget(QWidget *parent):
  QWidget(parent), d(new PackageWidgetData) {
  d->lib = 0;
  d->scale = 0.1;
  setFreeScaling(true);
  setAutoFillBackground(true);
}

PackageWidget::~PackageWidget() {
  delete d;
}

void PackageWidget::setFreeScaling(bool fs) {
  d->freescale = fs;
  if (fs)
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  else
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  update();
}

bool PackageWidget::isFreeScaling() const {
  return d->freescale;
}

void PackageWidget::setScale(double s) {
  d->scale = s;
  update();
}

double PackageWidget::scale() const {
  return d->scale;
}

void PackageWidget::setLibrary(class PackageLibrary const *lib) {
  d->lib = lib;
  update();
}
  
void PackageWidget::setPackage(QString name) {
  qDebug() << "setpackage" << name;
  d->name = name;
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
  if (drw.isValid()) {
    QPicture pic = drw.picture();
    QRect bb = pic.boundingRect();
    return (QSizeF(bb.size()) * d->scale / 10.0).toSize();
  } else {
    return QSize(10,10); // what can we do?
  }
}

void PackageWidget::paintEvent(QPaintEvent *) {
  QPainter ptr(this);
  
  PackageDrawing const &drw = d->drawing();

  if (drw.isValid()) {
    QPicture pic = drw.picture();
    QRect bb = pic.boundingRect();
    QSize s = size();
    double xr = s.width() * .8 / bb.width();
    double yr = s.height() * .8 / bb.height();
    double r = xr < yr ? xr : yr;
    if (!d->freescale) 
      if (r > d->scale)
	r = d->scale;
    QPoint p0((s.width() - r*bb.width())/2, (s.height() - r*bb.height())/2);
    ptr.scale(r, r);
    ptr.drawPicture(p0 - bb.topLeft(), pic);
  }
}

void PackageWidget::mousePressEvent(QMouseEvent *e) {
  e->accept();
  emit pressed(d->name);
}
