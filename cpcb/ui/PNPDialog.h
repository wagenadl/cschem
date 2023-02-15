// PNPDialog.h

#ifndef PNPDIALOG_H

#define PNPDIALOG_H

#include <QDialog>

class PNPDialog: public QDialog {
  Q_OBJECT;
public:
  PNPDialog(QWidget *parent=0);
  virtual ~PNPDialog();
  void setFolder(QString pwd);
  void setPNPFilename(QString fn);
  bool bomChecked() const;
  bool compactChecked() const;
  bool unplacedChecked() const;
  bool imageChecked() const;
  QString pnpFilename() const;
  QString unplacedFilename() const;
  QString bomFilename() const;
  QString imageFilename() const;
public slots:
  void browse();
private:
  class Ui_PNPDialog *ui;
  QString pwd;
};

#endif
