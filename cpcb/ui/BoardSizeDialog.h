// BoardSizeDialog.h

#ifndef BOARDSIZEDIALOG_H

#define BOARDSIZEDIALOG_H

#include "data/Dim.h"
#include "data/Rect.h"
#include "data/Board.h"
#include <QDialog>

class BoardSizeDialog: public QDialog {
  Q_OBJECT;
public:
  BoardSizeDialog(QWidget *parent=0);
  virtual ~BoardSizeDialog();
  void setLayout(class Layout const &);
  Dim boardWidth() const;
  Dim boardHeight() const;
  Board::Shape boardShape() const;
public slots:
  void shrink();
private:
  class Ui_BoardSizeDialog *ui;
  Rect minrect;
};

#endif
