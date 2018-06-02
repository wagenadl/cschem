// Propertiesbar.h

#ifndef PROPERTIESBAR_H

#define PROPERTIESBAR_H

#include <QToolBar>

class Propertiesbar: public QToolBar {
public:
  Propertiesbar(QWidget *parent=0);
private:
  class PBData *d;
};

#endif
