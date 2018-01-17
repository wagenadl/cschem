// LibView.h

#ifndef LIBVIEW_H

#define LIBVIEW_H

#include <QGraphicsView>

class LibView: public QGraphicsView {
  Q_OBJECT;
public:
  explicit LibView(QWidget *parent=0);
  explicit LibView(class SymbolLibrary const *lib, QWidget *parent=0);
  void scale(double);
  ~LibView();
signals:
  void activated(QString);
public slots:
  void clear();
  void rebuild(class SymbolLibrary const *lib=0);
public:
  void activate(QString); // causes the signal to be emitted
  friend class LibViewData;
private:
  class LibViewData *d;
};

#endif
