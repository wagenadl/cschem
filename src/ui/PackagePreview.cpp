// PackagePreview.cpp

#include "PackagePreview.h"
#include <QDebug>

class PackagePreviewData {
public:
  QString name;
};

PackagePreview::PackagePreview(QWidget *parent):
  QLabel(parent), d(new PackagePreviewData) {
}

PackagePreview::~PackagePreview() {
  delete d;
}

void PackagePreview::setPackage(QString name) {
  d->name = name;
  setText(name); // will set a pixmap, actually
  qDebug() << "setpackage" << name;
}
