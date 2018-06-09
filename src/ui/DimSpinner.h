// DimSpinner.h

#ifndef DIMSPINNER_H

#define DIMSPINNER_H

#include <QDoubleSpinBox>
#include "data/Dim.h"

class DimSpinner: public QDoubleSpinBox {
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
public slots:
  void setNoValue();
  void setValue(Dim, bool forceemit=false);
  void setMinimumValue(Dim);
  void setMaximumValue(Dim);
  void setMetric(bool b=true);
  void setInch();
  void setStep(Dim);
signals:
  void valueEdited(Dim); // by user interaction (mouse or keyboard)
private:
  double valueFromText(QString const &) const override;
  QString textFromValue(double) const override;
private:
  bool hasvalue_;
  bool metric_;
  int suppress_signals; 
  Dim minv, maxv;
  Dim step;
  friend class SupSig;
};

#endif
