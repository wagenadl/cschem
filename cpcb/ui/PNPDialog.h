// PNPDialog.h

#ifndef PNPDIALOG_H

#define PNPDIALOG_H

#include <QDialog>

class PNPDialog: public QDialog {
  
public:
  PNPDialog(QWidget *parent=0);
  virtual ~PNPDialog();
  bool bomChecked() const;
  bool compactChecked() const;
  bool imageChecked() const;
  QString filename() const;
public slots:
  void browse();
private:
  class Ui_PNPDialog *ui;
};

#endif
