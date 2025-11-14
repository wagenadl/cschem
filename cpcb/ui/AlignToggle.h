// AlignToggle.h

#ifndef ALIGNTOGGLE_H

#define ALIGNTOGGLE_H

#include <QToolButton>
#include "data/Rect.h"

class AlignToggle: public QToolButton {
  Q_OBJECT;
  enum Alignment {
    Center=0,
    Min=1, Left=1, Top=1,
    Max=2, Right=2, Bottom=2,
    Pin=3
  };
public:
  AlignToggle(Qt::Orientation o, QWidget *parent=0);
  virtual ~AlignToggle();
  void setOrientation(Qt::Orientation);
  void setAlignment(Alignment);
  Qt::Orientation orientation() const { return o; }
  Alignment alignment() const { return a; }
  Dim extractDimension(Rect const &r, Point const &pin);
  static Dim extractDimension(Rect const &r, Point const &pin,
                              Qt::Orientation o, Alignment a);
private:
  void updateIcon();
signals:
  void alignmentChanged(Alignment);
private slots:
  void click();
private:
  Qt::Orientation o;
  Alignment a;
};

#endif
