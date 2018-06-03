// Editor.h

#ifndef EDITOR_H

#define EDITOR_H

#include <QWidget>
#include "data/Layout.h"
#include "data/Point.h"

class Editor: public QWidget {
  Q_OBJECT;
public:
  Editor(QWidget *parent=0);
  virtual ~Editor();
  bool load(QString);
  bool save(QString) const; 
  Layout const &pcbLayout() const;
public slots:
  void setGrid(Dim);
  void setLayerVisibility(Layer, bool);
  void setPlanesVisibility(bool);
  void scaleToFit();
  void zoomIn();
  void zoomOut();
signals:
  void boardChanged();
  void hovering(Point);
  void leaving();
protected:
  void mousePressEvent(QMouseEvent *) override;
  void mouseMoveEvent(QMouseEvent *) override;
  void mouseReleaseEvent(QMouseEvent *) override;
  void keyPressEvent(QKeyEvent *) override;
  void enterEvent(QEvent *) override;
  void leaveEvent(QEvent *) override;
  void paintEvent(QPaintEvent *) override;
  void resizeEvent(QResizeEvent *) override;
private:
  class EData *d;
};

#endif
