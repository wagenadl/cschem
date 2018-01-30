// PackagePreview.h

#ifndef PACKAGEPREVIEW_H

#define PACKAGEPREVIEW_H

#include <QLabel>

class PackagePreview: public QLabel {
  Q_OBJECT;
public:
  PackagePreview(QWidget *parent=0);
  ~PackagePreview();
public slots:
  void setPackage(QString name);
private:
  class PackagePreviewData *d;
};

#endif
