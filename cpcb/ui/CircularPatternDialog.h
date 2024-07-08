// CircularPatternDialog.h

#ifndef CIRCULARPATTERNDIALOG_H

#define CIRCULARPATTERNDIALOG_H

#include <QDialog>

class CircularPatternDialog: public QDialog {
  Q_OBJECT;
public:
  static void gui(class Editor *editor, bool inc, QWidget *parent=0);
public:
  CircularPatternDialog(QWidget *parent=0);
  virtual ~CircularPatternDialog();
private:
  class Ui_CircularPatternDialog *ui;
};

#endif
