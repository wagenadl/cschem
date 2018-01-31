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
  QLabel(parent), d(new PackagePreviewData) {
  d->lib = 0;
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
  PackageDrawing drw;
  if (d->lib)
    drw = d->lib->drawing(name);
  else
    drw = PackageLibrary::defaultPackages().drawing(name);
  if (drw.isValid()) {
    QPicture pic = drw.picture();
    QRect bb = pic.boundingRect();
    QSize s = size();
    double xr = s.width() * 1.0 / bb.width();
    double yr = s.width() * 1.0 / bb.height();
    double r = xr < yr ? xr : yr;
    QPixmap pm(s);
    QPainter ptr;
    ptr.begin(&pm);
    ptr.scale(r, r);
    ptr.drawPicture(-bb.topLeft(), pic);
    ptr.end();
    setPixmap(pm);
  } else  {
    setText(name);
  }
}
