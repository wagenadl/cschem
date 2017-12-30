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
};  

class SceneConnectionData {
public:
  SceneConnectionData() {
    scene = 0;
    id = 0;
    hoverseg = 0;
    moveseg = -1;
    moveseg0 = 0;
    movesegN = 0;
  }
  QPolygonF path() const;
  bool danglingStart() const;
  bool danglingEnd() const;
  QPointF moveDelta(QPointF sp) const;
public:
  Scene *scene;
  QList<SCSegment *> segments;
  int id;
  SCSegment *hoverseg;
  QPointF movestart;
  QPolygonF origpath;
  SCSegment *moveseg0, *movesegN;
  int moveseg;
};

QPointF SceneConnectionData::moveDelta(QPointF sp) const {
  if (moveseg<0)
    return QPointF();
  QPointF delta = sp - movestart;
  QPointF ll = origpath[moveseg + 1] - origpath[moveseg];
  if (ll.isNull()) {
    // null line, don't know what to do exactly
  } else if (ll.x()==0) {
    // vertical line, cannot move in Y
    delta.setY(0);
  } else if (ll.y()==0) {
    // horizontal line, cannot move in X
    delta.setX(0);
  } else {
    // diag line, don't know what to do exactly
  }
  return delta;
}  

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
      addToGroup(d->segments.last());
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

void SceneConnection::mousePressEvent(QGraphicsSceneMouseEvent *e) {
  scene()->clearSelection();
  int seg = segmentAt(e->scenePos());
  d->moveseg = seg;
  if (seg>=0) {
    d->movestart = e->scenePos();
    d->origpath = d->path();
    if (seg==0) {
      d->moveseg0 = new SCSegment(this);
      addToGroup(d->moveseg0);
      d->moveseg0->setPen(QPen(QColor(0,0,0), 1.0,
                               Qt::SolidLine, Qt::SquareCap));
    } else {
      d->segments[seg-1]->setPen(QPen(QColor(0,0,0), 1.0,
                                      Qt::SolidLine, Qt::SquareCap));
    }
    if (seg==d->segments.size()-1) {
      d->movesegN = new SCSegment(this);
      addToGroup(d->movesegN);
      d->movesegN->setPen(QPen(QColor(0,0,0), 1.0,
                               Qt::SolidLine, Qt::SquareCap));
    } else {
      d->segments[seg+1]->setPen(QPen(QColor(0,0,0), 1.0,
                                      Qt::SolidLine, Qt::SquareCap));
    }      
    e->accept();
  }
}

void SceneConnection::mouseMoveEvent(QGraphicsSceneMouseEvent *e) {
  if (d->moveseg<0)
    return;

  QPointF delta = d->moveDelta(e->scenePos());
  QPointF p0 = d->moveseg - 1 < 0 ? d->origpath[0]
    : d->origpath[d->moveseg - 1];
  QPointF p1 = d->origpath[d->moveseg] + delta;
  QPointF p2 = d->origpath[d->moveseg + 1] + delta;
  QPointF p3 = d->moveseg + 2  >= d->origpath.size()
    ? d->origpath.last() : d->origpath[d->moveseg + 2];
  d->segments[d->moveseg]->setLine(QLineF(p1, p2));
  SCSegment *seg0 = d->moveseg0 ? d->moveseg0 : d->segments[d->moveseg - 1];
  seg0->setLine(QLineF(p0, p1));
  SCSegment *segN = d->movesegN ? d->movesegN : d->segments[d->moveseg + 1];
  segN->setLine(QLineF(p2, p3));
}

void SceneConnection::mouseReleaseEvent(QGraphicsSceneMouseEvent *e) {
  if (d->moveseg < 0)
    return;

  PartLibrary const *lib = d->scene->library();
  if (d->moveseg0)
    delete d->moveseg0;
  else
    d->segments[d->moveseg-1]->setPen(QPen(QColor(0,0,0), lib->lineWidth(),
                                           Qt::SolidLine, Qt::SquareCap));
  d->moveseg0 = 0;
  if (d->movesegN)
    delete d->movesegN;
  else
    d->segments[d->moveseg+1]->setPen(QPen(QColor(0,0,0), lib->lineWidth(),
                                           Qt::SolidLine, Qt::SquareCap));
    
  d->movesegN = 0;

  setPath(d->origpath);
  
  QPolygonF path = d->origpath;

  QPointF delta = d->moveDelta(e->scenePos());
  path[d->moveseg] += delta;
  path[d->moveseg+1] += delta;
  if (d->moveseg==0)
    path.prepend(d->origpath.first());
  if (d->moveseg==d->segments.size()-1)
    path.append(d->origpath.last());
  d->scene->modifyConnection(d->id, lib->simplifyPath(path));
}
