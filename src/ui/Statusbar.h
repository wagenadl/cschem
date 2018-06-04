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
public:
  Dim gridSpacing() const;
  bool isLayerVisible(Layer) const;
  bool arePlanesVisible() const;
public slots:
  void setCursorXY(Point);
  void hideCursorXY();
  void setGrid(Dim);
  void setBoard(Board const &);
  void hideLayer(Layer);
  void showLayer(Layer);
  void hidePlanes();
  void showPlanes();
  void resetGridChoices();
private:
  class QLabel *cursorui;
  QMap<Layer, class QToolButton *> layerui;
  QToolButton *planesui;
  class QComboBox *gridui;
  Board board;
  bool noemit;
};

#endif
