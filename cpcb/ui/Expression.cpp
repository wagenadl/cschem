// Expression.cpp

#include "Expression.h"
#include <QRegularExpression>
#include <QDebug>

enum class Unit {
  None, Inch, Mm,
};

class Value {
public:
  Value() {
    number = 0;
    unit = Unit::None;
    valid = false;
    anyunit = false;
  }
public:
  double number;
  Unit unit;
  bool valid;
  bool anyunit;
};

QDebug operator<<(QDebug d, Value v) {
  d << "value" << v.number
    << (v.unit==Unit::Mm ? "mm" : v.unit==Unit::Inch ? "inch" : "")
    << v.valid << v.anyunit;
  return d;
}

class ExprData {
public:
  ExprData() { }
  Value parseValue();
  Value parseFactor();
  Value parseTerm();
  Value parseExpression(bool toplevel=false);
  void skipWhite();
  bool atEnd() const;
public:
  Unit defaultUnit;
  Value value;
  QString str;
  Expression::Mode mode;
};

bool ExprData::atEnd() const {
  return str.isEmpty() || str.startsWith(")");
}

void ExprData::skipWhite() {
  while (str.startsWith(" "))
    str = str.mid(1);
}

Value ExprData::parseExpression(bool toplevel) {
  QString s = str;
  Value t1 = parseTerm();
  skipWhite();
  while (str.startsWith("+") || str.startsWith("-")) {
    bool isadd = str.startsWith("+");
    str = str.mid(1);
    skipWhite();
    Value t2 = parseTerm();
    if (!t1.valid || !t2.valid)
      return Value(); // invalid

    if (t1.unit!=t2.unit) {
      if (t1.unit==Unit::None) {
        if (toplevel && !t1.anyunit) {
          t1.unit = defaultUnit;
          t1.anyunit = true;
        } else {
          return Value(); // adding unitless to united is illegal
        }
      }
      if (t2.unit==Unit::None) {
        if (toplevel && !t2.anyunit) {
          t2.unit = defaultUnit;
          t2.anyunit = true;
        } else {
          return Value(); // adding unitless to united is illegal
        }
      }
    }

    t1.anyunit |= t2.anyunit;
    
    double num2 = t2.number;
    if (t1.unit==Unit::Mm && t2.unit==Unit::Inch)
      num2 *= 25.4;
    else if (t1.unit==Unit::Inch && t2.unit==Unit::Mm)
      num2 /= 25.4;
    if (isadd)
      t1.number += num2;
    else
      t1.number -= num2;
  }
  return t1;
}

Value ExprData::parseFactor() {
  static const QRegularExpression n1("^([0-9]*\\.?[0-9]*)\\s*(mm|dm|cm|m|um|\"|”|in|inch)?");
  static const QRegularExpression n2("[0-9]");
  QString s = str;

  if (str.startsWith("+"))
    str = str.mid(1);

  if (str.startsWith("-")) {
    str = str.mid(1);
    Value t1 = parseFactor();
    t1.number = -t1.number;
    return t1;
  }

  if (str.startsWith("(")) {
    str = str.mid(1);
    skipWhite();
    Value t1 = parseExpression();
    skipWhite();
    if (!str.startsWith(")"))
      return Value();
    str = str.mid(1);
    skipWhite();
    return t1;
  } 

  auto mtch = n1.match(str);
  if (mtch.hasMatch()) {
    bool ok;
    double num = mtch.captured(1).toDouble(&ok);
    if (!ok)
      return Value();
    QString unit = mtch.captured(2);
    str = str.mid(mtch.captured().length());
    skipWhite();
    Value v;
    v.valid = true;
    v.number = num;
    if (unit=="") {
      // no unit
      v.unit = Unit::None;
    } else if (unit.endsWith("m")) {
      // metric
      v.anyunit = true;
      v.unit = Unit::Mm;
      if (unit=="m")
        v.number *= 1000;
      else if (unit=="dm")
        v.number *= 100;
      else if (unit=="cm")
        v.number *= 10;
      else if (unit=="um")
        v.number /= 1000;
    } else {
      v.anyunit = true;
      v.unit = Unit::Inch;
    }
    return v;
  }
  return Value();
}

Value ExprData::parseTerm() {
  QString s = str;
  Value t1 = parseFactor();
  skipWhite();
  while (str.startsWith("*") || str.startsWith("/")) {
    bool ismul = str.startsWith("*");
    str = str.mid(1);
    skipWhite();
    Value t2 = parseFactor();
    if (!t1.valid || !t2.valid)
      return Value(); // invalid

    t1.anyunit |= t2.anyunit;
    
    double num2 = t2.number;
    
    if (ismul) {
      // multiply
      if (t1.unit!=Unit::None && t2.unit!=Unit::None)
        return Value(); // I won't do area units
      if (t1.unit==Unit::None)
        t1.unit=t2.unit;
      t1.number *= num2;
    } else {
      // divide
      if (num2==0)
        return Value(); // division by zero
      if (t1.unit==Unit::None && t2.unit!=Unit::None)
        return Value(); // I won't do inverse units
      if (t1.unit==Unit::Mm && t2.unit==Unit::Inch) // mm/inch -> unitless
        num2 *= 25.4;
      else if (t1.unit==Unit::Inch && t2.unit==Unit::Mm) // inch/mm -> unitless
        num2 /= 25.4;
      t1.number /= num2;
      if (t2.unit!=Unit::None)
        t1.unit = Unit::None;
    }
  }
  return t1;
}

Expression::Expression(): d(new ExprData) {
  d->defaultUnit = Unit::Inch;
  d->mode = Mode::ForceInch;
}

Expression::~Expression() {
  delete d;
}

void Expression::setMode(Expression::Mode m) {
  d->mode = m;
  d->defaultUnit = (m==Mode::ForceMetric || m==Mode::AutoMetric) ? Unit::Mm
    : (m==Mode::ForceInch || m==Mode::AutoInch) ? Unit::Inch
    : Unit::None;
}

void Expression::parse(QString str) {
  d->str = str;
  d->skipWhite();
  d->value = d->parseExpression(true);
  d->skipWhite();
  if (!d->str.isEmpty())
    d->value.valid = false;
  if (d->value.anyunit && d->value.unit==Unit::None)
    d->value.valid = false;
  
  if (d->mode==Mode::ForceInch && d->value.unit==Unit::Mm) {
    d->value.number /= 25.4;
    d->value.unit = Unit::Inch;
  } else if (d->mode==Mode::ForceMetric && d->value.unit==Unit::Inch) {
    d->value.number *= 25.4;
    d->value.unit = Unit::Mm;
  } else if (d->value.unit==Unit::None) {
    if (d->defaultUnit==Unit::None)
      d->value.valid = false;
    else
      d->value.unit = d->defaultUnit;
  }
}

bool Expression::isValid() const {
  return d->value.valid;
}

double Expression::value() const {
  return d->value.number;
}

bool Expression::isMetric() const {
  return d->value.unit == Unit::Mm;
}
