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
  bool saveAccordingly(class Layout const &pcblayout,
                       class LinkedSchematic const &schematic);
  QString filename() const;
public slots:
  void browse();
  void gerbernamechange();
private:
  bool saveGerber(class Layout const &pcblayout);
  bool saveBOM(class BOMTable const &bom);
  bool savePnP(class PickNPlace const &pnp);
  bool saveUnplaced(class PickNPlace const &pnp,
                    class BOMTable const &bom);     
  bool savePasteMask(class Layout const &pcblayout);
  bool saveFrontPanel(class Layout const &pcblayout);
  QString filename(QString ext) const;
private:
  class Ui_ExportDialog *ui;
  QString pwd;
};

#endif
