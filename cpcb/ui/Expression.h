// Expression.h

#ifndef EXPRESSION_H

#define EXPRESSION_H

/*
  EXPRESSION =
    TERM
    TERM + EXPRESSION
    TERM - EXPRESSION

  TERM =
    FACTOR
    FACTOR * TERM
    FACTOR / TERM

  FACTOR =
    VALUE
    ( EXPRESSION )

  VALUE =
    +VALUE
    -VALUE
    NUMBER [UNIT]

  NUMBER =
    /\.[0-9]+/
    /[0-9]+(\.[0-9]*)?/;

  UNIT =
    m
    cm
    mm
    um
    in
    inch
    "
    mil
*/

#include <QString>

class Expression {
public:
  enum class Mode {
    AutoInch,
    AutoMetric,
    ForceInch,
    ForceMetric,
    Explicit, // require explicit units; invalid otherwise
  };
public:
  Expression();
  ~Expression();
  void setMode(Mode);
  void parse(QString);
  bool isValid() const;
  double value() const;
  bool isMetric() const;
private:
  class ExprData *d;
  
};

#endif
