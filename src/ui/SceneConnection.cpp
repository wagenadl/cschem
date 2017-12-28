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
  QPolygonF path();
public:
  Scene *scene;
  int id;
};

QPolygonF SceneConnectionData::path() {
  Connection const &c = scene->circuit()->connections()[id];
  PartLibrary const *lib = scene->library();
  
  QPointF x0 = scene->pinPosition(c.fromId(), c.fromPin());
  QPointF x1 = scene->pinPosition(c.toId(), c.toPin());

  QPolygonF pp;
  pp << x0;
  for (QPoint p: c.via())
    pp << lib->scale()*p;
  pp << x1;
  
  return pp;
 }

SceneConnection::SceneConnection(class Scene *parent, Connection const &c) {
  d = new SceneConnectionData;

  d->scene = parent;
  d->id = c.id();

  PartLibrary const *lib = parent->library();

  rebuild();
  setPen(QPen(QColor(0,0,0), lib->scale()/2.0,
              Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
  parent->addItem(this);
}

void SceneConnection::rebuild() {
  auto path = d->path();
  QPainterPath pp(path.takeFirst());
  for (auto p: path)
    pp.lineTo(p);
  setPath(pp);
}

void SceneConnection::temporaryTranslate(QPointF delta) {
  auto path = d->path();
  path.translate(delta);
  QPainterPath pp(path.takeFirst());
  for (auto p: path)
    pp.lineTo(p);
  setPath(pp);
}

void SceneConnection::temporaryTranslateFrom(QPointF delta) {
  auto path = d->path();
  QPainterPath pp(path.takeFirst() + delta);
  for (auto p: path)
    pp.lineTo(p);
  setPath(pp);
}

void SceneConnection::temporaryTranslateTo(QPointF delta) {
  auto path = d->path();
  QPainterPath pp(path.takeFirst());
  path.last() += delta;
  for (auto p: path)
    pp.lineTo(p);
  setPath(pp);
}

SceneConnection::~SceneConnection() {
  delete d;
}

