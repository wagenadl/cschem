// ExportDialog.h

#ifndef EXPORTDIALOG_H

#define EXPORTDIALOG_H

#include <QDialog>

class ExportDialog: public QDialog {
  Q_OBJECT;
public:
  ExportDialog(QWidget *parent=0);
  virtual ~ExportDialog();
  bool runDialog(QString pcbfilename, QString exportdir);
  void saveAccordingly(class Layout const &pcblayout,
                       class LinkedSchematic const &schematic);
  QString filename() const;
private slots:
  void browse();
  void gerbernamechange();
private:
  class Ui_PNPDialog *ui;
  QString pwd;
};

#endif
