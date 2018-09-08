// TextTable.h

#ifndef TEXTTABLE_H

#define TEXTTABLE_H

#include <QTableWidget>

class TextTable: public QTableWidget {
public:
  TextTable(QWidget *parent=0);
  ~TextTable();
  TextTable(TextTable const &) = delete;
  TextTable &operator=(TextTable const &) = delete;
  QString text(int r, int c) const;
  void setText(int r, int c, QString txt);
  void setColumnHeader(int c, QString txt);
  void setRowHeader(int c, QString txt);
};

#endif
