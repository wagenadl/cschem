// BoardSizeDialog.cpp

#include "BoardSizeDialog.h"
#include "ui_BoardSizeDialog.h"
#include "DimSpinner.h"
#include "data/Layout.h"

BoardSizeDialog::BoardSizeDialog(QWidget *parent): QDialog(parent) {
  ui = new Ui_BoardSizeDialog;
  ui->setupUi(this);
  ui->margin->setValue(Dim::fromInch(0.050));
}
  
BoardSizeDialog::~BoardSizeDialog() {
  delete ui;
}

void BoardSizeDialog::shrink() {
  ui->width->setValue(minrect.right() + ui->margin->value());
  ui->height->setValue(minrect.bottom() + ui->margin->value());
}

void BoardSizeDialog::setLayout(class Layout const &lay) {
  minrect = lay.root().boundingRect();
  Board const &brd(lay.board());
  if (brd.isEffectivelyMetric()) {
    ui->width->setMetric(true);
    ui->height->setMetric(true);
  }
  ui->width->setValue(brd.width);
  ui->height->setValue(brd.height);
  ui->round->setChecked(brd.shape==Board::Shape::Round);
}

Board::Shape BoardSizeDialog::boardShape() const {
  return ui->round->isChecked()
    ? Board::Shape::Round
    : Board::Shape::Rect;
}

Dim BoardSizeDialog::boardWidth() const {
  return ui->width->value();
}

Dim BoardSizeDialog::boardHeight() const {
  return ui->height->value();
}

