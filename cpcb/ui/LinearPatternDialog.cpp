// LinearPatternDialog.cpp

#include "LinearPatternDialog.h"
#include "ui_LinearPatternDialog.h"
#include "Editor.h"

LinearPatternDialog::LinearPatternDialog(QWidget *parent): QDialog(parent) {
  ui = new Ui_LinearPatternDialog;
  ui->setupUi(this);
}

LinearPatternDialog::~LinearPatternDialog() {
  delete ui;
}

void LinearPatternDialog::gui(Editor *editor, bool metric, QWidget *parent) {
  LinearPatternDialog dlg(parent);
  dlg.ui->hspacing->setMetric(metric);
  dlg.ui->vspacing->setMetric(metric);
  dlg.ui->hspacing->setValue(metric ? Dim::fromMM(2.5) : Dim::fromInch(0.1));
  dlg.ui->vspacing->setValue(metric ? Dim::fromMM(2.5) : Dim::fromInch(0.1));

  if (!dlg.exec())
    return;

  int hcount = dlg.ui->horizontal->isChecked() ? dlg.ui->hcount->value() : 1;
  int vcount = dlg.ui->vertical->isChecked() ? dlg.ui->vcount->value() : 1;
  Dim hspacing = dlg.ui->hspacing->value();
  if (dlg.ui->left->isChecked())
    hspacing = -hspacing;
  Dim vspacing = dlg.ui->hspacing->value();
  if (dlg.ui->up->isChecked())
    vspacing = -vspacing;

  editor->linearPattern(hcount, hspacing, vcount, vspacing);
}
