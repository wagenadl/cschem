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
  void rebuild();
  void setLibrary(class SymbolLibrary const *lib);
public:
  void activate(QString); // causes the signal to be emitted
  friend class LibViewData;
private:
  class LibViewData *d;
};

#endif
