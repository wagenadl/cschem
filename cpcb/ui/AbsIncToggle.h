// AlignToggle.h

#ifndef ABSINCTOGGLE_H

#define ABSINCTOGGLE_H

#include <QToolButton>

class AbsIncToggle: public QToolButton {
  Q_OBJECT;
public:
  AbsIncToggle(QWidget *parent=0);
  virtual ~AbsIncToggle();
  void setInc(bool i=true);
  void setAbs();
  bool isInc() const { return inc; }
private:
  void updateIcon();
private:
  bool inc;
};

#endif
