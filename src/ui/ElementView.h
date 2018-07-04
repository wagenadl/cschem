// ElementView.h

#ifndef ELEMENTVIEW_H

#define ELEMENTVIEW_H

#include <QWidget>

class ElementView: public QWidget {
  Q_OBJECT;
public:
  ElementView(QWidget *parent=0);
  virtual ~ElementView();
  QString refText() const;
  QString pvText() const;
  class ComponentView const *component() const;
  class ComponentView *component();
public:
  static ElementView *instance(int); // search by ID
public slots:
  void setRefText(QString);
  void setPVText(QString);
private:
  void relabel();
private:
  class QLabel *l1, *l2;
  ComponentView *cv;
  QString ref;
  QString pv;
  static QMap<int, ElementView *> &cvmap();
};

#endif
