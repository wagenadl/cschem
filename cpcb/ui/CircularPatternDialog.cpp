// CircularPatternDialog.cpp

#include "CircularPatternDialog.h"
#include "ui_CircularPatternDialog.h"
#include "Editor.h"

CircularPatternDialog::CircularPatternDialog(QWidget *parent): QDialog(parent) {
  ui = new Ui_CircularPatternDialog;
  ui->setupUi(this);
}

CircularPatternDialog::~CircularPatternDialog() {
  delete ui;
}

void CircularPatternDialog::gui(Editor *editor, Point origin, QWidget *parent) {
  CircularPatternDialog dlg(parent);
  if (origin.isNull())
    dlg.ui->aroundOrigin->setText("Absolute origin");
  else
    dlg.ui->aroundOrigin->setText("Incremental origin");
  if (!dlg.exec())
    return;

  bool aroundOrigin = dlg.ui->aroundOrigin->isChecked(); // else sel'n center
  bool indiv = dlg.ui->individual->isChecked();
  int count = dlg.ui->count->value();
  bool specific = dlg.ui->spacingSpecific->isChecked(); // else even
  double deg = specific ? dlg.ui->spacing->value() : 360 / count;
  Point around = aroundOrigin ? origin
    : editor->selectionBounds().center();

  editor->circularPattern(count, FreeRotation(deg),
                          around, indiv);
}
