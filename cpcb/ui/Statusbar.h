// Statusbar.h

#ifndef STATUSBAR_H

#define STATUSBAR_H

#include <QStatusBar>
#include "data/Layer.h"
#include "data/Dim.h"
#include "data/Point.h"
#include "data/Board.h"
#include <QMap>

class Statusbar: public QStatusBar {
  Q_OBJECT;
public:
  Statusbar(QWidget *parent=0);
  virtual ~Statusbar();
signals:
  void gridEdited(Dim);
  void layerVisibilityEdited(Layer, bool);
  void planesVisibilityEdited(bool);
  void netsVisibilityEdited(bool);
public slots:
  void setCursorXY(Point);
  void setObject(QString);
  void setMissing(QStringList);
  void hideCursorXY();
  void setGrid(Dim);
  void setBoard(Board const &);
  void resetGridChoices();
  void setUserOrigin(Point);
private:
  void updateCursor();
private:
  class QLabel *cursorui;
  class QToolButton *missingui;
  QMap<Layer, class QToolButton *> layerui;
  QToolButton *planesui;
  QToolButton *netsui;
  class QComboBox *gridui;
  Board board;
  bool noemit;
  Point p;
  QString obj;
  Point ori;
  class DimSpinner *gridsp;
};

#endif
