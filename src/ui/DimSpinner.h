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
  void setValue(Dim);
  void setMinimumValue(Dim);
  void setMaximumValue(Dim);
  void setMetric(bool b=true);
  void setInch();
  void setStep(Dim);
signals:
  void valueChanged(Dim);
private:
  double valueFromText(QString const &) const override;
  QString textFromValue(double) const override;
private:
  bool hasvalue_;
  bool metric_;
  Dim minv, maxv;
  Dim step;
};

#endif
