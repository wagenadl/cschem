// DimSpinner.cpp

#include "DimSpinner.h"

DimSpinner::DimSpinner(QWidget *parent): QDoubleSpinBox(parent) {
  step = Dim::fromInch(.005);
  setInch();
  connect(this,
    static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
	  [this](double v) {
	    valueChanged(metric_ ? Dim::fromMM(v) : Dim::fromInch(v));
	  });
	  
}

Dim DimSpinner::value() const {
  if (metric_)
    return Dim::fromMM(QDoubleSpinBox::value());
  else
    return Dim::fromInch(QDoubleSpinBox::value());
}

bool DimSpinner::isMetric() const {
  return metric_;
}

bool DimSpinner::isInch() const {
  return !metric_;
}

void DimSpinner::setValue(Dim d) {
  if (metric_) 
    QDoubleSpinBox::setValue(d.toMM());
  else
    QDoubleSpinBox::setValue(d.toInch());
}

void DimSpinner::setStep(Dim d) {
  step = d;
  if (metric_) 
    QDoubleSpinBox::setSingleStep(d.toMM());
  else
    QDoubleSpinBox::setSingleStep(d.toInch());
}

void DimSpinner::setMetric(bool b) {
  if (b) {
    Dim d = value();
    metric_ = true;
    setSuffix(" mm");
    setDecimals(2);
    setStep(step);
    setValue(d);
  } else {
    setInch();
  }
}

void DimSpinner::setInch() {
  Dim d = value();
  metric_ = false;
  setSuffix("”");
  setDecimals(3);
  setStep(step);
  setValue(d);
}
