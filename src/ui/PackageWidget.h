// PackageWidget.h

#ifndef PACKAGEWIDGET_H

#define PACKAGEWIDGET_H

#include <QWidget>

class PackageWidget: public QWidget {
  Q_OBJECT;
public:
  PackageWidget(QWidget *parent=0);
  ~PackageWidget();
  double scale() const; // returns scale in pixels per 0.001". Default is 0.1.
  bool isFreeScaling() const; // true means: will expand to available space
  QSize sizeHint() const override;
  QSize minimumSizeHint() const override;
  int margin() const; // ideally, determined by grid spacing
  double gridSpacing() const;
signals:
  void pressed(QString); // argument is name of package
public slots:
  void setPackage(QString name);
  void setLibrary(class PackageLibrary const *lib);
  void setScale(double pix_per_mil);
  void setFreeScaling(bool);
protected:
  void paintEvent(QPaintEvent *) override;
  void mousePressEvent(QMouseEvent *) override;
  void enterEvent(QEvent *) override;
  void leaveEvent(QEvent *) override;
private:
  class PackageWidgetData *d;
};

#endif
