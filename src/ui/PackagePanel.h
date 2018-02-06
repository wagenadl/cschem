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
signals:
  void pressed(QString); // argument is name of package
public slots:
  void setElement(class Element const &elt);
  void clear();
  void setLibrary(class PackageLibrary const *lib);
  void setScale(double pix_per_mil);
private slots:
  void press(QString);
private:
  class PackagePanelData *d;
};

#endif
