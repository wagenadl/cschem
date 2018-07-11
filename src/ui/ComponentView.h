// ComponentView.h

#ifndef COMPONENTVIEW_H

#define COMPONENTVIEW_H

#include <QWidget>
#include "data/Group.h"

class ComponentView: public QWidget {
  Q_OBJECT;
public:
  static constexpr char const *dndformat = "application/x-dnd-cpcb-componentview";
public:
  ComponentView(QWidget *parent=0);
  virtual ~ComponentView();
public:
  int rotation() const;
  bool isFlipped() const;
  Group const &group() const;
  double scale() const;
  int id() const;
public slots:
  void setGroup(Group const &);
  void setRotation(int);
  void setFlipped(bool);
  void setScale(double pxPerMil);
signals:
  void edited(); // user has rotated, flipped, or changed the group by dropping
  // not emitted upon setGroup, setRotation, or setFlipped.
  void changed(); // also emitted upon setGroup, setRotation, and setFlipped.
public:
  QSize sizeHint() const override;
  QSize minimumSizeHint() const override;
protected:
  void keyPressEvent(QKeyEvent *) override;
  void mousePressEvent(QMouseEvent *) override;
  void mouseMoveEvent(QMouseEvent *) override;
  void mouseReleaseEvent(QMouseEvent *) override;
  void dragEnterEvent(QDragEnterEvent *) override;
  void dragMoveEvent(QDragMoveEvent *) override;
  void dragLeaveEvent(QDragLeaveEvent *) override;
  void dropEvent(QDropEvent *) override;
  void paintEvent(QPaintEvent *) override;
private:
  Group grp;
  int rot;
  bool flp;
  double mil2px;
  QPoint presspt;
  int id_;
private:
  static int idgen();
};

#endif
