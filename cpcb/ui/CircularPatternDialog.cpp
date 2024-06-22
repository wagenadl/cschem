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

void CircularPatternDialog::gui(Editor *editor, bool inc, QWidget *parent) {
  CircularPatternDialog dlg(parent);
  if (inc)
    dlg.ui->aroundOrigin->setText("Incremental origin");
  else
    dlg.ui->aroundOrigin->setText("Absolute origin");
  if (!dlg.exec())
    return;

  bool aroundOrigin = dlg.ui->aroundOrigin->isChecked(); // else sel'n center
  bool indiv = dlg.ui->individual->isChecked();
  int count = dlg.ui->count->value();
  bool specific = dlg.ui->spacingSpecific->isChecked(); // else even
  double deg = specific ? dlg.ui->spacing->value() : 360 / count;
  Point around;
  if (aroundOrigin) {
    if (inc)
      around = editor->userOrigin();
  } else {
    around = editor->selectionBounds().center();
  }
  editor->circularPattern(count, FreeRotation(deg),
                          around, indiv);
}
