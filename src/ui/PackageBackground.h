// PackageBackground.h

#ifndef PACKAGEBACKGROUND_H

#define PACKAGEBACKGROUND_H

#include <QWidget>

class PackageBackground: public QWidget {
public:
  PackageBackground(QWidget *parent=0);
  void setScale(double pixpermil);
  void setGridSpacing(double mil, int majorival);
  double scale() const { return scale_; }
  double gridSpacing() const { return grid; }
  int majorInterval() const { return major; }
  QPoint nearestGridIntersection(QPoint) const;
protected:
  void paintEvent(QPaintEvent *) override;
private:
  double scale_;
  double grid;
  int major;
};

#endif
