// SceneConnection.cpp

#include "SceneConnection.h"
#include <QGraphicsPathItem>
#include "file/Connection.h"
#include "svg/PartLibrary.h"
#include "Scene.h"
#include <QDebug>

#define SIMPLETEMP 0

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
  setPen(QPen(QColor(0,0,0), lib->lineWidth(),
              Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
  parent->addItem(this);
}

void SceneConnection::rebuild() {
  auto path = d->path();
  QPainterPath pp(path.takeFirst());
  for (auto p: path)
    pp.lineTo(p);
  setPath(pp);
  setLineWidth();
}

void SceneConnection::temporaryTranslate(QPointF delta) {
  auto path = d->path();
  path.translate(delta);
  QPainterPath pp(path.takeFirst());
  for (auto p: path)
    pp.lineTo(p);
  setPath(pp);
  setLineWidth(.5);
}

void SceneConnection::setLineWidth(double frac) {
  PartLibrary const *lib = scene()->library();
  QPen p = pen();
  p.setWidthF(lib->lineWidth() *frac);
  setPen(p);
}

void SceneConnection::temporaryTranslateFrom(QPointF delta) {
  auto path = d->path();
  QPainterPath pp(path.takeFirst() + delta);
#if SIMPLETEMP
  pp.lineTo(path.last());
#else
  for (auto p: path)
    pp.lineTo(p);
#endif
  setPath(pp);
  setLineWidth(.5);
}

void SceneConnection::temporaryTranslateTo(QPointF delta) {
  auto path = d->path();
  QPainterPath pp(path.takeFirst());
  path.last() += delta;
#if SIMPLETEMP
  pp.lineTo(path.last());
#else
  for (auto p: path)
    pp.lineTo(p);
#endif
  setPath(pp);
  setLineWidth(.5);
}

SceneConnection::~SceneConnection() {
  delete d;
}


Scene *SceneConnection::scene() {
  return d->scene;
}

int SceneConnection::id() const {
  return d->id;
}

