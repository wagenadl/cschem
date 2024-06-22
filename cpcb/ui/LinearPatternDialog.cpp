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

  int hcount = dlg.ui->hcount->value();
  int vcount = dlg.ui->vcount->value();
  Dim hspacing = dlg.ui->hspacing->value();
  Dim vspacing = dlg.ui->hspacing->value();

  editor->linearPattern(hcount, hspacing, vcount, vspacing);
}
