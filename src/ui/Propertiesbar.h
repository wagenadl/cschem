// Propertiesbar.h

#ifndef PROPERTIESBAR_H

#define PROPERTIESBAR_H

class Propertiesbar: public QToolbar {
public:
  Propertiesbar(QWidget *parent=0);
private:
  class PBData *d;
};

#endif
