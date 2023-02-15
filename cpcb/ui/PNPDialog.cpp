// PNPDialog.cpp

#include "PNPDialog.h"
#include "ui_PNPDialog.h"
#include <QFileInfo>
#include <QFileDialog>

PNPDialog::PNPDialog(QWidget *parent): QDialog(parent) {
  ui = new Ui_PNPDialog;
  ui->setupUi(this);
}

PNPDialog::~PNPDialog() {
  delete ui;
}

bool PNPDialog::bomChecked() const {
  return ui->bom->isChecked();
}

bool PNPDialog::compactChecked() const {
  return ui->compact->isChecked();
}

bool PNPDialog::unplacedChecked() const {
  return ui->unplaced->isChecked();
}

bool PNPDialog::imageChecked() const {
  return ui->image->isChecked();
}

QString PNPDialog::pnpFilename() const {
  return QFileInfo(ui->filename->text()).absolutePath();
}


QString PNPDialog::unplacedFilename() const {
  QFileInfo fi(ui->filename->text());
  return fi.absolutePath() + "/" + fi.baseName() + ".unplaced.csv";
}
QString PNPDialog::bomFilename() const {
  QFileInfo fi(ui->filename->text());
  return fi.absolutePath() + "/" + fi.baseName() + ".bom.csv";
}

QString PNPDialog::imageFilename() const {
  QFileInfo fi(ui->filename->text());
  return fi.absolutePath() + "/" + fi.baseName() + ".pnp.png";
}

void PNPDialog::setPNPFilename(QString fn) {
  QFileInfo fi(fn);
  if (fn.isEmpty())
    ui->filename->setText("");
  else
    ui->filename->setText(fi.absolutePath()
                          + "/" + fi.baseName() + ".pnp.csv");
}

void PNPDialog::setFolder(QString pwd0) {
  pwd = pwd0;
}

void PNPDialog::browse() {
  QString fn = ui->filename->text();
  QString sug = fn.isEmpty() ? pwd : fn;
  fn = QFileDialog::getSaveFileName(this, "PNP filename…",
                                    sug,
                                    "PNP files (*.pnp.csv");
  if (!fn.isEmpty())
    setPNPFilename(fn);
}

