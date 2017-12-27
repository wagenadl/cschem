// Scene.h

#ifndef SCENE_H

#define SCENE_H

#include <QGraphicsScene>
#include <QGraphicsItem>
#include "file/Circuit.h"
#include "svg/PartLibrary.h"

class Scene: public QGraphicsScene {
public:
  Scene(PartLibrary const &lib, QObject *parent=0);
  void setCircuit(Circuit *);
  void rebuild();
private:
  void createElement(int id, QPoint pos, QString sym);
  void createConnection(Connection const &);
  QPoint pinPosition(int partid, QString pin) const;
private:
  PartLibrary const &lib;
  Circuit *circuit;
  QMap<int, QGraphicsItem *> items;
};

#endif
