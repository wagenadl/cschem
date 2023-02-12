// PNPDialog.cpp

#include "PNPDialog.h"
#include "ui_PNPDialog.h"

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


bool PNPDialog::imageChecked() const {
  return ui->image->isChecked();
}

QString PNPDialog::filename() const {
  return ui->filename->text();
}
