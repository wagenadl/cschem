// SceneConnection.cpp

#include "SceneConnection.h"
#include <QGraphicsLineItem>
#include "file/Connection.h"
#include "svg/PartLibrary.h"
#include "Scene.h"
#include <QDebug>
#include <QGraphicsColorizeEffect>
#include <QGraphicsSceneHoverEvent>
#include "Style.h"

class SCSegment: public QGraphicsLineItem {
public:
  static const int UNDERLINGWIDTH = 10;
  SCSegment(QGraphicsItem *parent=0):
    QGraphicsLineItem(parent) {
    underling = new QGraphicsLineItem(this);
    underling->setZValue(-100);
    underling->setPen(QPen(QColor(255,255,255, 0), UNDERLINGWIDTH,
			   Qt::SolidLine, Qt::FlatCap));
    underling->setFlag(ItemStacksBehindParent);
  }
  SCSegment(QPointF p0, QGraphicsItem *parent=0):
    SCSegment(parent) {
    setLine(QLineF(p0, p0));
  }
  ~SCSegment() {
    //    delete underling;
  }
  SCSegment(SCSegment const &) = delete;
  SCSegment operator=(SCSegment const &) = delete;
  void setLine(QLineF const &l) {
    QGraphicsLineItem::setLine(l);
    underling->setLine(l);
  }
  QRectF effectiveBoundingRect() const {
    return boundingRect() | underling->boundingRect();
  }
private:
  QGraphicsLineItem *underling;
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
  QPointF moveDelta(QPointF sp) const;
  QPen normalPen() const;
  QPen draftPen() const;
  bool isDangling() const;
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

bool SceneConnectionData::isDangling() const {
  return scene->circuit().connection(id).isDangling();
}

QPen SceneConnectionData::normalPen() const {
  return QPen(isDangling() ? Style::danglingColor() : Style::connectionColor(),
	      scene->library()->lineWidth(),
	      Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
}

QPen SceneConnectionData::draftPen() const {
  return QPen(isDangling() ? Style::danglingColor() : Style::connectionColor(),
	      Style::connectionDraftWidthFactor()
	      * scene->library()->lineWidth(),
	      Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
}

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

QPolygonF SceneConnectionData::path() const {
  Connection const &c = scene->circuit().connection(id);
  PartLibrary const *lib = scene->library();

  QPolygonF pp;
  if (!c.danglingStart())
    pp << scene->pinPosition(c.fromId(), c.fromPin());
  for (QPoint p: c.via())
    pp << lib->scale()*p;
  if (!c.danglingEnd())
    pp << scene->pinPosition(c.toId(), c.toPin());
  
  return pp;
 }

SceneConnection::SceneConnection(class Scene *parent, Connection const &c):
  d(new SceneConnectionData) {

  d->scene = parent;
  d->id = c.id();

  rebuild();
  parent->addItem(this);
}

void SceneConnection::setPath(QPolygonF const &path) {
  while (d->segments.size() > path.size() - 1)
    delete d->segments.takeLast();

  for (int k=1; k<path.size(); k++) {
    if (k-1 >= d->segments.size()) {
      d->segments << new SCSegment(this);
      addToGroup(d->segments.last());
    }
    auto *seg = d->segments[k-1];
    seg->setLine(QLineF(path[k-1], path[k]));
    seg->setPen(d->normalPen());
  }
}

void SceneConnection::rebuild() {
  setPath(d->path());
  setLineWidth();
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
  //  delete d;
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
    if (d->segments[k]->effectiveBoundingRect().contains(p))
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
  d->hoverseg->setPen(QPen(Style::connectionHoverColor(),
			   Style::connectionHoverWidthFactor()*lib->lineWidth(),
                           Qt::SolidLine, Qt::RoundCap));
  d->hoverseg->setZValue(-1);

  
}

void SceneConnection::unhover() {
  if (!d->hoverseg)
    return;
  if (d->segments.contains(d->hoverseg)) {
    d->hoverseg->setPen(d->normalPen());
    d->hoverseg->setZValue(0);
  }
  d->hoverseg = 0;
}

void SceneConnection::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e) {
  /* Simplify if possible */
  qDebug() << "SceneConnection::mouseDoubleClickEvent";
  scene()->clearSelection();
  int seg = segmentAt(e->scenePos());
  scene()->simplifySegment(d->id, seg);
  d->moveseg = -1;
  e->accept();
}  

void SceneConnection::mousePressEvent(QGraphicsSceneMouseEvent *e) {
  qDebug() << "SceneConnection::mousePressEvent";
  scene()->clearSelection();
  int seg = segmentAt(e->scenePos());
  d->moveseg = seg;
  QPen draftPen(d->draftPen());
  if (seg>=0) {
    d->movestart = e->scenePos();
    d->origpath = d->path();
    qDebug() << "origpath" << d->origpath;
    if (seg==0) {
      if (!d->scene->circuit().connection(d->id).danglingStart()) {
	d->moveseg0 = new SCSegment(d->origpath.first(), this);
	addToGroup(d->moveseg0);
	d->moveseg0->setPen(draftPen);
      }
    } else {
      d->segments[seg-1]->setPen(draftPen);
    }
    if (seg==d->segments.size()-1) {
      if (!d->scene->circuit().connection(d->id).danglingEnd()) {
	d->movesegN = new SCSegment(d->origpath.last(), this);
	addToGroup(d->movesegN);
	d->movesegN->setPen(draftPen);
      }
    } else {
      d->segments[seg+1]->setPen(draftPen);
    }      
    e->accept();
  }
}

void SceneConnection::mouseMoveEvent(QGraphicsSceneMouseEvent *e) {
  qDebug() << "SceneConnection::mouseMoveEvent";
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
  SCSegment *seg0 = d->moveseg - 1 < 0 ? d->moveseg0
    : d->segments[d->moveseg - 1];
  if (seg0)
    seg0->setLine(QLineF(p0, p1));
  SCSegment *segN = d->moveseg + 2 >= d->origpath.size() ? d->movesegN
    : d->segments[d->moveseg + 1];
  if (segN)
    segN->setLine(QLineF(p2, p3));
}

void SceneConnection::mouseReleaseEvent(QGraphicsSceneMouseEvent *e) {
  qDebug() << "SceneConnection::mouseReleaseEvent";
  if (d->moveseg < 0)
    return;

  PartLibrary const *lib = d->scene->library();

  if (d->moveseg0)
    delete d->moveseg0;
  d->moveseg0 = 0;
  if (d->movesegN)
    delete d->movesegN;
  d->movesegN = 0;

  setPath(d->origpath);
  
  QPolygonF path = d->origpath;

  QPointF delta = d->moveDelta(e->scenePos());
  path[d->moveseg] += delta;
  path[d->moveseg+1] += delta;
  if (d->moveseg==0
      && !d->scene->circuit().connection(d->id).danglingStart())
    path.prepend(d->origpath.first());
  if (d->moveseg==d->segments.size()-1
      && !d->scene->circuit().connection(d->id).danglingEnd())
    path.append(d->origpath.last());
  d->scene->modifyConnection(d->id, lib->simplifyPath(path));
}

SceneConnection::WeakPtr::WeakPtr(SceneConnection *s,
				  QSharedPointer<SceneConnectionData> const &d):
  s(s), d(d) {
}

SceneConnection::WeakPtr::WeakPtr(): s(0) {
}

SceneConnection *SceneConnection::WeakPtr::data() const {
  return d.isNull() ? 0 : s;
}

void SceneConnection::WeakPtr::clear() {
  d.clear();
}

SceneConnection::WeakPtr::operator bool() const {
  return !d.isNull();
}


SceneConnection::WeakPtr SceneConnection::weakref() {
  return SceneConnection::WeakPtr(this, d);
}
