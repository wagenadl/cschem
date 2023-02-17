// ExportDialog.cpp

#include "ExportDialog.h"
#include "ui_ExportDialog.h"
#include <QFileInfo>
#include <QFileDialog>

ExportDialog::ExportDialog(QWidget *parent): QDialog(parent) {
  ui = new Ui_ExportDialog;
  ui->setupUi(this);
}

ExportDialog::~ExportDialog() {
  delete ui;
}

bool ExportDialog::runDialog(QString pcbfilename, QString exportdir) {
  pwd = exportdir;
  QFileInfo fi(pcbfilename);
  if (pcbfilename.isEmpty())
    ui->gerberfilename->setText("");
  else
    ui->gerberfilename->setText(fi.absolutePath()
                                + "/" + fi.baseName() + ".zip");
  return exec();
}

QString ExportDialog::filename() const {
  return ui->gerberfilename->text();
}

void ExportDialog::browse() {
  QString fn = ui->gerberfilename->text();
  QString sug = fn.isEmpty() ? pwd : fn;
  fn = QFileDialog::getSaveFileName(this, "Gerber filename…",
                                    sug,
                                    "Gerber files (*.zip");
  if (!fn.isEmpty())
    ui->gerberfilename->setText(fn);
}



void ExportDialog::gerbernamechange() {
  QString fn = ui->gerberfilename->text();
  QFileInfo fi(fn);
  if (fn.isEmpty()) {
    ui->bomfilename->setText("");
    ui->pnpfilename->setText("");
    ui->unplacedfilename->setText("");
    ui->imagefilename->setText("");
  } else {
    QString base = fn.baseName();
    ui->bomfilename->setText(".../" + base + "-bom.csv");
    ui->pnpfilename->setText(".../" + base + "-pnp.csv");
    ui->unplacedfilename->setText(".../" + base + "unplaced.csv");
    ui->imagefilename->setText(".../" + base + "-pnpimage.png");
  }
}

bool ExportDialog::saveAccordingly(class Layout const &pcblayout,
                                   class LinkedSchematic const &schematic) {
}
