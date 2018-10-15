// DimSpinner.h

#ifndef DIMSPINNER_H

#define DIMSPINNER_H

#include <QLineEdit>
#include "data/Dim.h"

class DimSpinner: public QLineEdit {
  Q_OBJECT;
public:
  DimSpinner(QWidget *parent=0);
  virtual ~DimSpinner();
  Dim value() const;
  bool isMetric() const;
  bool isInch() const;
  bool hasValue() const;
  Dim minimumValue() const;
  Dim maximumValue() const;
  QSize sizeHint() const override;
  QSize minimumSizeHint() const override;
public slots:
  void setNoValue();
  void setValue(Dim, bool forceemit=false);
  void setMinimumValue(Dim);
  void setMaximumValue(Dim);
  void setMetric(bool b=true);
  void setInch();
  void setStep(Dim);
protected:
  void focusOutEvent(QFocusEvent *) override;
  void keyPressEvent(QKeyEvent *) override;
signals:
  void valueEdited(Dim); // by user interaction (mouse or keyboard)
private:
  void parseValue();
  void reflectValue();
  void reflectValid(bool);
private:
  bool hasvalue_;
  bool metric_;
  int suppress_signals; 
  Dim v, minv, maxv;
  Dim step;
  friend class SupSig;
};

#endif
