// SceneConnection.cpp

#include "SceneConnection.h"
#include <QGraphicsLineItem>
#include "file/Connection.h"
#include "svg/PartLibrary.h"
#include "Scene.h"
#include <QDebug>
#include <QGraphicsColorizeEffect>
#include <QGraphicsSceneHoverEvent>

class SCSegment: public QGraphicsLineItem {
public:
  SCSegment(QGraphicsItem *parent=0): QGraphicsLineItem(parent) {
    // setFlag(ItemIsMovable);
    setCacheMode(DeviceCoordinateCache);
  }
  void mousePressEvent(QGraphicsSceneMouseEvent *e) override {
    scene()->clearSelection();
    qDebug() << "SCSegment mouse press" << e->scenePos() << e->pos();
    QGraphicsLineItem::mousePressEvent(e);
  }
  void mouseMoveEvent(QGraphicsSceneMouseEvent *e) override {
    qDebug() << "SCSegment mouse move" << e->scenePos() << e->pos();
    QGraphicsLineItem::mouseMoveEvent(e);
  }
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *e) override {
    qDebug() << "SCSegment mouse release" << e->scenePos() << e->pos();
    QGraphicsLineItem::mouseReleaseEvent(e);
  }
};  

class SceneConnectionData {
public:
  SceneConnectionData() {
    scene = 0;
    id = 0;
    hoverseg = 0;
  }
  QPolygonF path() const;
  bool danglingStart() const;
  bool danglingEnd() const;
public:
  Scene *scene;
  QList<SCSegment *> segments;
  int id;
  SCSegment *hoverseg;
};

bool SceneConnectionData::danglingStart() const {
  Connection const &c = scene->circuit().connections()[id];
  return c.fromId() <= 0;
}

bool SceneConnectionData::danglingEnd() const {
  Connection const &c = scene->circuit().connections()[id];
  return c.toId() <= 0;
}

QPolygonF SceneConnectionData::path() const {
  Connection const &c = scene->circuit().connections()[id];
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

  rebuild();
  parent->addItem(this);
}

void SceneConnection::setPath(QPolygonF const &path) {
  PartLibrary const *lib = d->scene->library();

  while (d->segments.size() > path.size() - 1)
    delete d->segments.takeLast();

  for (int k=1; k<path.size(); k++) {
    if (k-1 >= d->segments.size()) {
      d->segments << new SCSegment(this);
    }
    auto *seg = d->segments[k-1];
    seg->setLine(QLineF(path[k-1], path[k]));
    seg->setPen(QPen(QColor(0,0,0), lib->lineWidth(),
		     Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
  }
}

void SceneConnection::rebuild() {
  setPath(d->path());
  setLineWidth();
  if (d->danglingStart())
    d->segments.first()->hide();
  else
    d->segments.first()->show();
  if (d->danglingEnd())
    d->segments.last()->hide();
  else
    d->segments.last()->show();
}

void SceneConnection::temporaryTranslate(QPointF delta) {
  auto path = d->path();
  path.translate(delta);
  setPath(path);
  setLineWidth(.5);
}

void SceneConnection::setLineWidth(double frac) {
  if (d->segments.isEmpty())
    return;
  PartLibrary const *lib = scene()->library();
  QPen p = d->segments.first()->pen();
  p.setWidthF(lib->lineWidth() *frac);
  for (auto *seg: d->segments)
    seg->setPen(p);
}

void SceneConnection::temporaryTranslateFrom(QPointF delta) {
  auto path = d->path();
  path.first() += delta;
  setPath(path);
  setLineWidth(.5);
}

void SceneConnection::temporaryTranslateTo(QPointF delta) {
  auto path = d->path();
  path.last() += delta;
  setPath(path);
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

void SceneConnection::paint(QPainter *, QStyleOptionGraphicsItem const *,
                            QWidget *) {
}

QRectF SceneConnection::boundingRect() const {
  return QRectF();
}

int SceneConnection::segmentAt(QPointF p) const {
  for (int k=0; k<d->segments.size(); k++)
    if (d->segments[k]->boundingRect().contains(p))
      return k;
  return -1;
}

void SceneConnection::hover(int seg) {
  if (seg<0 || seg>=d->segments.size())
    return;
  if (d->segments[seg] == d->hoverseg)
    return;
  unhover();

  d->hoverseg = d->segments[seg];

  PartLibrary const *lib = d->scene->library();
  d->hoverseg->setPen(QPen(QColor(64, 192, 255), 3*lib->lineWidth(),
                           Qt::SolidLine, Qt::RoundCap));

  
}

void SceneConnection::unhover() {
  if (!d->hoverseg)
    return;
  
  PartLibrary const *lib = d->scene->library();
  d->hoverseg->setPen(QPen(QColor(0, 0, 0), lib->lineWidth(),
                           Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
  d->hoverseg = 0;
}
