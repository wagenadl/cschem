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
  double gridSpacing() const;
  int gridInterval() const;
  bool arePinsVisible() const;
public:
  QSize sizeHint() const override;
  QSize minimumSizeHint() const override;
signals:
  void pressed(QString); // argument is name of package
public slots:
  void setPackage(QString name);
  void setLibrary(class PackageLibrary const *lib);
  void setScale(double pix_per_mil);
  void setGrid(double mil, int major=5);
  void setPinsVisible(bool);
protected:
  void paintEvent(QPaintEvent *) override;
  void mousePressEvent(QMouseEvent *) override;
  void enterEvent(QEvent *) override;
  void leaveEvent(QEvent *) override;
private:
  class PackageWidgetData *d;
};

#endif
