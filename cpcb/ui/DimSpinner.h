// DimSpinner.h

#ifndef DIMSPINNER_H

#define DIMSPINNER_H

#include <QLineEdit>
#include "data/Dim.h"
#include "Expression.h"

class DimSpinner: public QLineEdit {
  Q_OBJECT;
public:
  DimSpinner(QWidget *parent=0);
  virtual ~DimSpinner();
  Dim value() const;
  bool isMetric() const;
  bool isInch() const;
  bool hasValue() const;
  bool isValid() const;
  Dim minimumValue() const;
  Dim maximumValue() const;
  QSize sizeHint() const override;
  QSize minimumSizeHint() const override;
public slots:
  void setNoValue();
  void setNoValueText(QString);
  void setValue(Dim, bool forceemit=false);
  void setMinimumValue(Dim);
  void setMaximumValue(Dim);
  void setMetric(bool b=true);
  void setInch();
  void setMode(Expression::Mode);
  void setStep(Dim);
  void parseValue();
  void showTrailingZeros();
  void hideTrailingZeros();
protected:
  void focusOutEvent(QFocusEvent *) override;
  void keyPressEvent(QKeyEvent *) override;
signals:
  void valueEdited(Dim); // by user interaction (mouse or keyboard)
private:
  void reflectValue();
  void reflectValid(bool);
private:
  bool hidetrz_;
  bool hasvalue_;
  bool valid_;
  bool metric_;
  Expression::Mode mode_;
  int suppress_signals; 
  Dim v, minv, maxv;
  Dim step;
  QString nvtext;
};

#endif
