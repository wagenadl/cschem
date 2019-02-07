// SignalNameCombo.h

#ifndef SIGNALNAMECOMBO_H

#define SIGNALNAMECOMBO_H

#include <QComboBox>

class SignalNameCombo: public QComboBox {
public:
  SignalNameCombo(class Symbol const &sym, QWidget *parent=0);
  void setCurrent(QString);
  ~SignalNameCombo();
};

#endif
