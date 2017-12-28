// SceneConnection.cpp

#include "SceneConnection.h"
#include <QGraphicsPathItem>
#include "file/Connection.h"
#include "svg/PartLibrary.h"
#include "Scene.h"
#include <QDebug>

class SceneConnectionData {
public:
  SceneConnectionData() {
    scene = 0;
    id = 0;
  }
public:
  Scene *scene;
  int id;
};

SceneConnection::SceneConnection(class Scene *parent, Connection const &c) {
  d = new SceneConnectionData;
  //
  d->scene = parent;
  d->id = c.id();

  PartLibrary const *lib = parent->library();

  rebuild();
  setPen(QPen(QColor(0,0,0), lib->scale()/2.0,
              Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
  parent->addItem(this);
}

void SceneConnection::rebuild() {
  Connection const &c = d->scene->circuit()->connections()[d->id];
  PartLibrary const *lib = d->scene->library();
  
  QPointF x0 = d->scene->pinPosition(c.fromId(), c.fromPin());
  QPointF x1 = d->scene->pinPosition(c.toId(), c.toPin());

  QPainterPath pp(x0);
  for (QPoint p: c.via())
    pp.lineTo(lib->scale()*p);
  pp.lineTo(x1);
  setPath(pp);
  
}

SceneConnection::~SceneConnection() {
  delete d;
}


