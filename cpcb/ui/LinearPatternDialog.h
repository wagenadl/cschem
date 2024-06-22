// LinearPatternDialog.h

#ifndef LINEARPATTERNDIALOG_H

#define LINEARPATTERNDIALOG_H

#include <QDialog>

class LinearPatternDialog: public QDialog {
  Q_OBJECT;
public:
  static void gui(class Editor *editor, bool metric, QWidget *parent=0);
public:
  LinearPatternDialog(QWidget *parent=0);
  virtual ~LinearPatternDialog();
private:
  class Ui_LinearPatternDialog *ui;
};

#endif
