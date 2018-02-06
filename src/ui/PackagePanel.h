// PackagePanel.h

#ifndef PACKAGEPANEL_H

#define PACKAGEPANEL_H

#include <QScrollArea>

class PackagePanel: public QScrollArea {
  Q_OBJECT;
public:
  PackagePanel(QWidget *parent=0);
  virtual ~PackagePanel();
  double scale() const; // returns scale in pixels per 0.001". Default is 0.1.
  bool isFreeScaling() const; // true means: will expand to available space
signals:
  void pressed(QString); // argument is name of package
public slots:
  void setElement(class Element const &elt);
  void clear();
  void setLibrary(class PackageLibrary const *lib);
  void setScale(double pix_per_mil);
  void setFreeScaling(bool);
private:
  class PackagePanelData *d;
};

#endif
