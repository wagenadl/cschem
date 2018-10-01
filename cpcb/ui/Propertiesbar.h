// Propertiesbar.h

#ifndef PROPERTIESBAR_H

#define PROPERTIESBAR_H

#include <QToolBar>
#include "Mode.h"
#include "data/Group.h"

class Propertiesbar: public QToolBar {
  Q_OBJECT;
public:
  Propertiesbar(class Editor *editor, QWidget *parent=0);
public slots:
  void reflectMode(Mode);
  void reflectSelection();
  void reflectBoard(class Board const &);
  void forwardAllProperties();
  void stepPinNumber();
  void setUserOrigin(Point);
private:
  class PBData *d;
};

#endif
