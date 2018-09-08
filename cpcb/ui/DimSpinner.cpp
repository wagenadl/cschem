// DimSpinner.cpp

#include "DimSpinner.h"

class SupSig {
public:
  SupSig(DimSpinner *ds, bool fake=false): ds(ds), fake(fake) {
    if (!fake)
      ds->suppress_signals++;
  }
  ~SupSig() {
    if (!fake)
      ds->suppress_signals--;
  }
private:
  DimSpinner *ds;
  bool fake;
};

DimSpinner::DimSpinner(QWidget *parent): QDoubleSpinBox(parent) {
  metric_ = false;
  hasvalue_ = true;
  suppress_signals = 0;
  minv = Dim::fromInch(0.);
  maxv = Dim::fromInch(100.);
  step = Dim::fromInch(.005);
  setInch();
  connect(this,
    static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
	  [this](double v) {
	    if (!hasvalue_ && hasValue()) {
	      // meaning, we *now* have a value
	      hasvalue_ = true;
	      setMinimumValue(minv);
	    }
	    if (suppress_signals==0)
	      valueEdited(metric_ ? Dim::fromMM(v) : Dim::fromInch(v));
	  });
}

DimSpinner::~DimSpinner() {
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

void DimSpinner::setNoValue() {
  SupSig ss(this);
  hasvalue_ = false;
  setMinimumValue(minv);
  QDoubleSpinBox::setSpecialValueText("---");
  QDoubleSpinBox::setValue(QDoubleSpinBox::minimum());
}

bool DimSpinner::hasValue() const  {
  return hasvalue_ ? true : value() > minv - step/2;
}

void DimSpinner::setValue(Dim d, bool doemit) {
  SupSig ss(this, doemit);
  if (!hasvalue_) {
    hasvalue_ = true;
    QDoubleSpinBox::setSpecialValueText(QString());
    setMinimumValue(minv);
  }
  if (metric_) 
    QDoubleSpinBox::setValue(d.toMM());
  else
    QDoubleSpinBox::setValue(d.toInch());
}

void DimSpinner::setStep(Dim d) {
  SupSig ss(this);
  step = d;
  QDoubleSpinBox::setSingleStep(metric_ ? d.toMM() : d.toInch());
}

void DimSpinner::setMinimumValue(Dim d) {
  SupSig ss(this);
  minv = d;
  double mv = metric_ ? minv.toMM() : minv.toInch();
  if (hasvalue_) 
    QDoubleSpinBox::setMinimum(mv);
  else
    QDoubleSpinBox::setMinimum(mv - QDoubleSpinBox::singleStep());
}

void DimSpinner::setMaximumValue(Dim d) {
  SupSig ss(this);
  maxv = d;
  double mv = metric_ ? maxv.toMM() : maxv.toInch();
  QDoubleSpinBox::setMaximum(mv);
}

void DimSpinner::setMetric(bool b) {
  SupSig ss(this);
  if (b) {
    bool ok = hasValue();
    Dim d = value();
    metric_ = true;
    setSuffix(" mm");
    setDecimals(2);
    setStep(step);
    setMinimumValue(minv);
    setMaximumValue(maxv);
    if (ok)
      setValue(d);
  } else {
    setInch();
  }
}

void DimSpinner::setInch() {
  SupSig ss(this);
  Dim d = value();
  bool ok = hasValue();
  metric_ = false;
  setSuffix("”");
  setDecimals(3);
  setStep(step);
  setMinimumValue(minv);
  setMaximumValue(maxv);
  if (ok)
    setValue(d);
}

double DimSpinner::valueFromText(QString const &s) const {
  return QDoubleSpinBox::valueFromText(s);
}

QString DimSpinner::textFromValue(double d) const {
  return QDoubleSpinBox::textFromValue(d);
}
