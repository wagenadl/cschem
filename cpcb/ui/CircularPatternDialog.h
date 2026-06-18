// CircularPatternDialog.h

#ifndef CIRCULARPATTERNDIALOG_H

#define CIRCULARPATTERNDIALOG_H

#include <QDialog>
#include "data/Point.h"

class CircularPatternDialog: public QDialog {
  Q_OBJECT;
public:
  static void gui(class Editor *editor, Point origin, QWidget *parent=0);
public:
  CircularPatternDialog(QWidget *parent=0);
  virtual ~CircularPatternDialog();
private:
  class Ui_CircularPatternDialog *ui;
};

#endif
