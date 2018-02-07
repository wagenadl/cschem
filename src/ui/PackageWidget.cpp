// PackageWidget.cpp

#include "PackageWidget.h"
#include "svg/PackageLibrary.h"
#include "svg/PackageDrawing.h"
#include "PackageBackground.h"

#include <QPainter>
#include <QDebug>
#include <QMouseEvent>

class PackageWidgetData {
public:
  PackageWidgetData() {
    lib = 0;
    freescale = false;
    scale = 0.3;
    mousein = false;
  }
  PackageDrawing const &drawing();
public:
  PackageLibrary const *lib;
  QString name;
  bool freescale;
  double scale;
  bool mousein;
};

PackageWidget::PackageWidget(QWidget *parent):
  QWidget(parent), d(new PackageWidgetData) {
  setFreeScaling(false);
}

PackageWidget::~PackageWidget() {
  delete d;
}

void PackageWidget::setFreeScaling(bool fs) {
  d->freescale = fs;
  if (fs)
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  else
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  update();
}

bool PackageWidget::isFreeScaling() const {
  return d->freescale;
}

void PackageWidget::setScale(double s) {
  d->scale = s;
  updateGeometry();
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
  PackageBackground *bg = dynamic_cast<PackageBackground *>(parent());
  PackageDrawing drw = d->drawing();
  if (drw.isValid()) {
    QPicture pic = drw.picture();
    QRect bb = pic.boundingRect();
    QSizeF bbs = QSizeF(bb.size());
    double delta = bg ? bg->gridSpacing() : 100;
    bbs += 2.2*QSizeF(delta, delta);
    return (bbs * d->scale).toSize();
  } else {
    return QSize(10,10); // what can we do?
  }
}

QSize PackageWidget::minimumSizeHint() const {
  PackageDrawing drw = d->drawing();
  if (drw.isValid()) {
    QPicture pic = drw.picture();
    QRect bb = pic.boundingRect();
    return (QSizeF(bb.size()) * d->scale).toSize() + QSize(8, 8);
  } else {
    return QSize(10,10); // what can we do?
  }
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
  PackageBackground *bg = dynamic_cast<PackageBackground *>(parent());
  QPainter ptr(this);

  if (d->mousein) 
    ptr.fillRect(QRect(QPoint(0,0), size()), QColor(0, 128, 255, 64));
  
  PackageDrawing const &drw = d->drawing();

  if (drw.isValid()) {
    QPicture pic = drw.picture();
    QRect bb = pic.boundingRect();
    QSize s = size();
    double xr = (s.width() - 8) * 1. / bb.width();
    double yr = (s.height() - 8) * 1. / bb.height();
    double r = xr < yr ? xr : yr;
    if (!d->freescale) 
      if (r > d->scale)
	r = d->scale;
    /* Instead of the following, we should align to background */
    ptr.translate(s.width()/2, s.height()/2);
    ptr.scale(r, r);
    ptr.drawPicture(-bb.center(), pic);
  }
}

void PackageWidget::mousePressEvent(QMouseEvent *e) {
  e->accept();
  emit pressed(d->name);
}
