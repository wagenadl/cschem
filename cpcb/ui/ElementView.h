// ElementView.h

#ifndef ELEMENTVIEW_H

#define ELEMENTVIEW_H

#include "ComponentView.h"

class ElementView: public ComponentView {
  Q_OBJECT;
public:
  ElementView(QWidget *parent=0);
  virtual ~ElementView();
  QString refText() const;
  QString pvText() const;
public:
  static ElementView *instance(int); // search by ID
public slots:
  void setRefText(QString);
  void setPVText(QString);
  void linkEditor(class Editor *);
  void pasteOutlineFromEditor();
protected:
  void paintEvent(QPaintEvent *) override;
  void mousePressEvent(QMouseEvent *) override;
private:
  void relabel();
private:
  QString ref;
  QString pv;
  static QMap<int, ElementView *> &cvmap();
  QString reflbl;
  QString pvlbl;
  class Editor *ed;
};

#endif
