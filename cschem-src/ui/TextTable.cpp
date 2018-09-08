// TextTable.cpp

#include "TextTable.h"

TextTable::TextTable(QWidget *parent): QTableWidget(parent) {
}

TextTable::~TextTable() {
}

QString TextTable::text(int r, int c) const {
  auto *it = item(r, c);
  if (it)
    return it->text();
  else
    return "";
}

void TextTable::setText(int r, int c, QString txt) {
  auto *it = item(r, c);
  if (it)
    it->setText(txt);
  else
    setItem(r, c, new QTableWidgetItem(txt));
}

void TextTable::setColumnHeader(int c, QString txt) {
  setHorizontalHeaderItem(c, new QTableWidgetItem(txt));
}

void TextTable::setRowHeader(int r, QString txt) {
  setVerticalHeaderItem(r, new QTableWidgetItem(txt));
}


  
