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

double PackageWidget::gridSpacing() const {
  PackageBackground *bg = dynamic_cast<PackageBackground *>(parent());
  if (bg)
    return bg->gridSpacing();
  else
    return 100.;
}

int PackageWidget::margin() const {
  return round(.5 * gridSpacing() * d->scale);
}


QSize PackageWidget::sizeHint() const {
  PackageDrawing drw = d->drawing();
  if (drw.isValid()) {
    QPicture pic = drw.picture();
    QRect bb = pic.boundingRect();
    QSizeF bbs = QSizeF(bb.size());
    double grid = gridSpacing();
    double delta = margin();
    QSize hint = (bbs * d->scale).toSize() + 2*QSize(delta, delta);
    int h0 = round(d->scale * grid * 2.6);
    if (hint.height() < h0)
      hint.setHeight(h0);
    return hint;
  } else {
    return QSize(10,10); // what can we do?
  }
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
  QWidget *bg0 = parentWidget();
  PackageBackground *bg = dynamic_cast<PackageBackground *>(parentWidget());
  qDebug() << "PW: parent" << parent() << bg;
  QPainter ptr(this);

  PackageDrawing const &drw = d->drawing();

  if (drw.isValid()) {
    QPicture pic = drw.picture();
    QRect bb = pic.boundingRect();
    QSize s = size();
    int mrg = margin();
    double xr = (s.width() - 2*mrg) * 1. / bb.width();
    double yr = (s.height() - 2*mrg) * 1. / bb.height();
    double r = xr < yr ? xr : yr;
    if (!d->freescale) 
      if (r > d->scale)
	r = d->scale;
    QTransform xf;
    xf.translate(s.width()/2, s.height()/2);
    xf.scale(r, r);
    qDebug() << "scale" << r << bg << drw.pins().size();
    xf.translate(-bb.center().x(), -bb.center().y());
    // xf maps picture coords to widget coords s.t. center goes to center
    if (bg && !drw.pins().isEmpty()) {
      PackageDrawing::PinInfo pin1 = drw.pins().begin().value();
      QPoint pos1 = pin1.position;
      QPoint wpos1 = mapToParent(xf.map(pos1));
      QPoint wgpos1 = bg->nearestGridIntersection(wpos1);
      QPoint gpos1 = xf.inverted().map(mapFromParent(wgpos1));
      QPoint delta = gpos1 -pos1;
      qDebug() << pos1 << wpos1 << ">" << wgpos1 << gpos1 << " >" << delta << r;
      xf.translate(delta.x(), delta.y()); // translate s.t. pin ends up on grid
    }
    ptr.setTransform(xf);      
    if (d->mousein) 
      ptr.fillRect(bb.adjusted(-50, -50, 50, 50), QColor(0, 128, 255, 64));
    ptr.drawPicture(QPoint(0,0), pic);
  }
}

void PackageWidget::mousePressEvent(QMouseEvent *e) {
  e->accept();
  emit pressed(d->name);
}
