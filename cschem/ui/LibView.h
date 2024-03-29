// LibView.h

#ifndef LIBVIEW_H

#define LIBVIEW_H

#include <QGraphicsView>

class LibView: public QGraphicsView {
  Q_OBJECT;
public:
  explicit LibView(QWidget *parent=0);
  void scale(double);
  ~LibView();
signals:
  void activated(QString, QString);
  void hoveron(QString, QString);
  void hoveroff();
public slots:
  void clear();
  void rebuild();
  void setLibrary(class SymbolLibrary const *lib);
public:
  void resizeEvent(QResizeEvent *e) override;
  void activate(QString, QString); // causes the signal to be emitted
  void hover(QString, QString);
  void unhover();
  friend class LibViewData;
private:
  class LibViewData *d;
};

#endif
