// PackagePreview.h

#ifndef PACKAGEPREVIEW_H

#define PACKAGEPREVIEW_H

#include <QWidget>

class PackagePreview: public QWidget {
  Q_OBJECT;
public:
  PackagePreview(QWidget *parent=0);
  ~PackagePreview();
public slots:
  void setPackage(QString name);
  void setLibrary(class PackageLibrary *lib);
protected:
  void paintEvent(QPaintEvent *) override;
private:
  class PackagePreviewData *d;
};

#endif
