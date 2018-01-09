// LibView.h

#ifndef LIBVIEW_H

#define LIBVIEW_H

#include <QGraphicsView>

class LibView: public QGraphicsView {
  Q_OBJECT;
public:
  explicit LibView(QWidget *parent=0);
  explicit LibView(class PartLibrary const *lib, QWidget *parent=0);
  void rebuild(class PartLibrary const *lib);
  void scale(double);
  ~LibView();
signals:
  void activated(QString);
public:
  void activate(QString); // causes the signal to be emitted
  friend class LibViewData;
private:
  class LibViewData *d;
};

#endif
