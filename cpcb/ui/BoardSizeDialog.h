// BoardSizeDialog.h

#ifndef BOARDSIZEDIALOG_H

#define BOARDSIZEDIALOG_H

#include "data/Dim.h"
#include "data/Rect.h"
#include "data/Board.h"
#include <QDialog>

class BoardSizeDialog: public QDialog {
public:
  BoardSizeDialog(QWidget *parent=0);
  ~BoardSizeDialog();
  void setLayout(class Layout const &);
  Dim boardWidth() const;
  Dim boardHeight() const;
  Board::Shape boardShape() const;
private:
  class Ui_BoardSizeDialog *ui;
  Rect minrect;
};

#endif
