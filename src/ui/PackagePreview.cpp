// PackagePreview.cpp

#include "PackagePreview.h"
#include "svg/PackageLibrary.h"
#include "svg/PackageDrawing.h"

#include  <QPainter>
#include <QDebug>

class PackagePreviewData {
public:
  PackageLibrary const *lib;
  QString name;
};

PackagePreview::PackagePreview(QWidget *parent):
  QWidget(parent), d(new PackagePreviewData) {
  d->lib = 0;
  setAutoFillBackground(true);
}

PackagePreview::~PackagePreview() {
  delete d;
}

void PackagePreview::setLibrary(class PackageLibrary *lib) {
  d->lib = lib;
}
  
void PackagePreview::setPackage(QString name) {
  qDebug() << "setpackage" << name;
  d->name = name;
}

void PackagePreview::paintEvent(QPaintEvent *) {
  QPainter ptr(this);
  
  PackageDrawing drw;
  if (d->lib)
    drw = d->lib->drawing(d->name);
  else
    drw = PackageLibrary::defaultPackages().drawing(d->name);

  if (drw.isValid()) {
    QPicture pic = drw.picture();
    QRect bb = pic.boundingRect();
    QSize s = size();
    double xr = s.width() * .8 / bb.width();
    double yr = s.height() * .8 / bb.height();
    double r = xr < yr ? xr : yr;
    QPoint p0((s.width() - r*bb.width())/2, (s.height() - r*bb.height())/2);
    ptr.scale(r, r);
    ptr.drawPicture(p0 - bb.topLeft(), pic);
  }
}
