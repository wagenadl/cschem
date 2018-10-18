// DimSpinner.cpp

#include "DimSpinner.h"
#include "Expression.h"
#include <QFontMetrics>
#include <QKeyEvent>

DimSpinner::DimSpinner(QWidget *parent): QLineEdit(parent) {
  nvtext = "---";
  metric_ = false;
  hidetrz_ = false;
  mode_ = Expression::Mode::ForceInch;
  hasvalue_ = true;
  valid_ = true;
  minv = Dim::fromInch(-100.);
  maxv = Dim::fromInch(100.);
  step = Dim::fromInch(.005);
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  //  setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  QPalette p(palette());
  //p.setColor(QPalette::Text, QColor(0, 0, 0));
  setPalette(p);
  setInch();
  connect(this, &QLineEdit::returnPressed,
	  [this]() {
            parseValue();
	  });
  connect(this, &QLineEdit::textEdited,
          [this]() {
            Expression expr;
            expr.setMode(mode_);
            expr.parse(text());
            reflectValid(expr.isValid());
          });            
}

DimSpinner::~DimSpinner() {
}

void DimSpinner::keyPressEvent(QKeyEvent *e) {
  if (e->key()==Qt::Key_Escape) {
    Expression expr;
    expr.setMode(mode_);
    expr.parse(text());
    reflectValue();
    reflectValid(true);
    if (expr.isValid())
      clearFocus();
  } else {
    QLineEdit::keyPressEvent(e);
  }
}

void DimSpinner::focusOutEvent(QFocusEvent *e) {
  QLineEdit::focusOutEvent(e);
  parseValue();
}

Dim DimSpinner::value() const {
  return hasValue() ? v : Dim();
}

bool DimSpinner::isMetric() const {
  return metric_;
}

bool DimSpinner::isInch() const {
  return !metric_;
}

void DimSpinner::setNoValue() {
  hasvalue_ = false;
  reflectValid(true);
  reflectValue();
}

bool DimSpinner::hasValue() const  {
  return hasvalue_;
}

void DimSpinner::setValue(Dim d, bool doemit) {
  hasvalue_ = true;
  v = d;
  reflectValue();
  reflectValid(true);
  if (doemit)
    emit valueEdited(v);
}

void DimSpinner::setStep(Dim d) {
  step = d;
}

void DimSpinner::setMinimumValue(Dim d) {
  minv = d;
  parseValue();
}

void DimSpinner::setMaximumValue(Dim d) {
  maxv = d;
  parseValue();
}

void DimSpinner::setMode(Expression::Mode m) {
  mode_ = m;
  if (mode_==Expression::Mode::ForceMetric)
    metric_ = true;
  else if (mode_==Expression::Mode::ForceInch)
    metric_ = false;
  reflectValue();
  updateGeometry();
}

void DimSpinner::setMetric(bool b) {
  metric_ = b;
  mode_ = b ? Expression::Mode::ForceMetric : Expression::Mode::ForceInch;
  reflectValue();
  updateGeometry();
}

void DimSpinner::setInch() {
  setMetric(false);
}

void DimSpinner::reflectValue() {
  // show value in our text box
  if (hasvalue_) {
    if (hidetrz_) {
      if (metric_)
	setText(QString("%1 mm").arg(v.toMM()));
      else
	setText(QString("%1”").arg(v.toInch()));
    } else {
      if (metric_)
	setText(QString("%1 mm").arg(v.toMM(), 0, 'f', 2, QChar('0')));
      else
	setText(QString("%1”").arg(v.toInch(), 0, 'f', 3, QChar('0')));
    }
  } else {
    setText(nvtext);
  }
}

void DimSpinner::setNoValueText(QString s) {
  nvtext = s;
  reflectValue();
}

void DimSpinner::parseValue() {
  if (text() == nvtext) {
    hasvalue_ = false;
    reflectValid(true);
    return;
  }
  Expression expr;
  expr.setMode(mode_);
  expr.parse(text());
  if (expr.isValid()) {
    reflectValid(true);
    metric_ = expr.isMetric();
    Dim v1 = metric_ ? Dim::fromMM(expr.value())
      : Dim::fromInch(expr.value());
    if (v1<minv)
      v1 = minv;
    else if (v1>maxv)
      v1 = maxv;
    if (v1 != v || hasvalue_) {
      hasvalue_ = true;
      v = v1;
      reflectValue();
      emit valueEdited(v);
    }
  } else {
    // invalid
    reflectValid(false);
  }
}

bool DimSpinner::isValid() const {
  return valid_;
}

void DimSpinner::reflectValid(bool val) {
  valid_ = val;
  QPalette p(palette());
  QColor col(val ? QColor(0, 0, 0) : QColor(255, 0, 0));
  p.setColor(QPalette::Normal, QPalette::Text, col);
  p.setColor(QPalette::Inactive, QPalette::Text, col);
  setPalette(p);
}

QSize DimSpinner::sizeHint() const {
  QFontMetrics fm(font());
  QString txt = metric_ ? "-000.00 mm" : "-00.000”";
  QSize s = fm.boundingRect(txt).size();
  return QSize(10*s.width()/9, 5*s.height()/4);
}

QSize DimSpinner::minimumSizeHint() const {
  QFontMetrics fm(font());
  QString txt = metric_ ? "0.00 mm" : "0.000”";
  QSize s = fm.boundingRect(txt).size();
  return QSize(s.width(), 5*s.height()/4);
}

void DimSpinner::showTrailingZeros() {
  hidetrz_ = false;
  reflectValue();
}

void DimSpinner::hideTrailingZeros() {
  hidetrz_ = true;
  reflectValue();
}
