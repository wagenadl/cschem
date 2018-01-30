// SceneConnection.cpp

#include "SceneConnection.h"
#include <QGraphicsLineItem>
#include "circuit/Connection.h"
#include "svg/SymbolLibrary.h"
#include "Scene.h"
#include <QDebug>
#include <QGraphicsColorizeEffect>
#include <QGraphicsSceneHoverEvent>
#include "Style.h"
#include <math.h>

class SCSegment: public QGraphicsLineItem {
public:
  static const int UNDERLINGWIDTH = 10;
  SCSegment(SceneConnection *parent=0):
    QGraphicsLineItem(parent) {
    underling = new QGraphicsLineItem(this);
    underling->setZValue(-100);
    underling->setPen(QPen(QColor(255,255,255, 0), UNDERLINGWIDTH,
			   Qt::SolidLine, Qt::FlatCap));
    underling->setFlag(ItemStacksBehindParent);
  }
  SCSegment(QPointF p0, SceneConnection *parent=0):
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
  SceneConnectionData(SceneConnection *conn, Scene *scene):
    conn(conn), scene(scene) {
    id = 0;
    hoverseg = 0;
    moveseg = -1;
    moveseg0 = 0;
    movesegN = 0;
    reallymoving = false;
  }
  QPolygonF path() const;
  QPointF moveDelta(QPointF sp, bool nomagnet=false);
  QPen normalPen() const;
  QPen draftPen() const;
  bool isDangling() const;
  void startRealMove();
  void updateMove(QPointF delta);
public:
  SceneConnection *conn;
  Scene *scene;
  QList<SCSegment *> segments;
  int id;
  SCSegment *hoverseg;
  QPointF movestart;
  QPolygonF origpath;
  SCSegment *moveseg0, *movesegN;
  int moveseg;
  bool reallymoving;
  bool havemagnet;
  double magnetdelta;
};

bool SceneConnectionData::isDangling() const {
  return scene->circuit().connection(id).isDangling();
}

QPen SceneConnectionData::normalPen() const {
  return QPen(isDangling() ? Style::danglingColor() : Style::layerColor(),
	      scene->library().lineWidth(),
	      Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
}

QPen SceneConnectionData::draftPen() const {
  return QPen(isDangling() ? Style::danglingColor() : Style::layerColor(),
	      Style::connectionDraftWidthFactor()
	      * scene->library().lineWidth(),
	      Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
}

QPointF SceneConnectionData::moveDelta(QPointF sp, bool nomagnet) {
  if (moveseg<0)
    return QPointF();
  double s = scene->library().scale();
  QPointF delta = scene->library().nearestGrid(sp - movestart);
  if (nomagnet)
    return delta;
  QPointF ll = origpath[moveseg + 1] - origpath[moveseg];
  if (ll.isNull()) {
    // null line, don't know what to do exactly
  } else if (ll.x()==0) {
    // vertical line, cannot move in Y
    delta.setY(0);
    if (havemagnet) {
      if (fabs(delta.x()-magnetdelta) < 3*s)
        delta.setX(magnetdelta);
      else
        havemagnet = false;
    }
    double x0 = origpath[moveseg].x();
    double x1 = x0 + delta.x();
    if (moveseg<=0) {
      if (fabs(delta.x()) < 2*s) {
        magnetdelta = 0;
        delta.setX(0);
        havemagnet = true;
      }
    } else {
      double xpre = origpath[moveseg-1].x();
      if (fabs(x1 - xpre) < 2*s) {
        magnetdelta = xpre - x0;
        delta.setX(magnetdelta);
        havemagnet = true;
      }
    }
    if (moveseg+2>=origpath.size()) {
      if (fabs(delta.x()) < 2*s) {
        magnetdelta = 0;
        delta.setX(0);
        havemagnet = true;
      }
    } else {
      double xpost = origpath[moveseg+2].x();
      if (fabs(x1 - xpost) < 2*s) {
        magnetdelta = xpost - x0;
        delta.setX(magnetdelta);
        havemagnet = true;
      }
    }
  } else if (ll.y()==0) {
    // horizontal line, cannot move in X
    delta.setX(0);
    if (havemagnet) {
      if (fabs(delta.y()-magnetdelta) < 3*s)
        delta.setY(magnetdelta);
      else
        havemagnet = false;
    }
    double y0 = origpath[moveseg].y();
    double y1 = y0 + delta.y();
    if (moveseg<=0) {
      if (fabs(delta.y()) < 2*s) {
        magnetdelta = 0;
        delta.setY(0);
        havemagnet = true;
      }
    } else {
      double ypre = origpath[moveseg-1].y();
      if (fabs(y1 - ypre) < 2*s) {
        magnetdelta = ypre - y0;
        delta.setY(magnetdelta);
        havemagnet = true;
      }
    }
    if (moveseg+2>=origpath.size()) {
      if (fabs(delta.y()) < 2*s) {
        magnetdelta = 0;
        delta.setY(0);
        havemagnet = true;
      }
    } else {
      double ypost = origpath[moveseg+2].y();
      if (fabs(y1 - ypost) < 2*s) {
        magnetdelta = ypost - y0;
        delta.setY(magnetdelta);
        havemagnet = true;
      }
    }
  } else {
    // diag line, don't know what to do exactly
  }
  return delta;
}

QPolygonF SceneConnectionData::path() const {
  Connection const &c = scene->circuit().connection(id);
  SymbolLibrary const &lib = scene->library();

  QPolygonF pp;
  if (!c.danglingStart())
    pp << scene->pinPosition(c.fromId(), c.fromPin());
  for (QPoint p: c.via())
    pp << lib.scale()*p;
  if (!c.danglingEnd())
    pp << scene->pinPosition(c.toId(), c.toPin());
  qDebug() << "Connection" << c.report() << pp;
  return pp;
 }

SceneConnection::SceneConnection(class Scene *parent, Connection const &c):
  d(new SceneConnectionData(this, parent)) {
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
    QLineF line(path[k-1], path[k]);
    seg->setLine(line);
    if (line.length()==0)
      seg->hide();
    else
      seg->show();
    seg->setPen(d->normalPen());
  }
}

void SceneConnection::rebuild() {
  setPath(d->path());
  setLineWidth();
}

void SceneConnection::temporaryTranslate(QPoint delta) {
  auto path = d->path();
  path.translate(d->scene->library().upscale(delta));
  setPath(path);
  setLineWidth(.5);
}

void SceneConnection::setLineWidth(double frac) {
  if (d->segments.isEmpty())
    return;
  SymbolLibrary const &lib = scene()->library();
  QPen p = d->segments.first()->pen();
  p.setWidthF(lib.lineWidth() *frac);
  for (auto *seg: d->segments)
    seg->setPen(p);
}

void SceneConnection::temporaryTranslateFrom(QPoint delta) {
  auto path = d->path();
  path.first() += d->scene->library().upscale(delta);
  setPath(path);
  setLineWidth(.5);
}

void SceneConnection::temporaryTranslateTo(QPoint delta) {
  auto path = d->path();
  path.last() += d->scene->library().upscale(delta);
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

  SymbolLibrary const &lib = d->scene->library();
  d->hoverseg->setPen(QPen(Style::hoverColor(),
			   Style::connectionHoverWidthFactor()*lib.lineWidth(),
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
  d->movestart = e->scenePos();
  d->origpath = d->path();
  d->reallymoving = false;
  d->havemagnet = false;
  e->accept();
}

void SceneConnectionData::startRealMove() {
  QPen pen(draftPen());
  origpath = path();
  qDebug() << "origpath" << origpath;
  if (moveseg==0) {
    if (!scene->circuit().connection(id).danglingStart()) {
      moveseg0 = new SCSegment(origpath.first(), conn);
      conn->addToGroup(moveseg0);
      moveseg0->setPen(pen);
    }
  } else {
    segments[moveseg-1]->setPen(pen);
  }
  if (moveseg==segments.size()-1) {
    if (!scene->circuit().connection(id).danglingEnd()) {
      movesegN = new SCSegment(origpath.last(), conn);
      conn->addToGroup(movesegN);
      movesegN->setPen(pen);
    }
  } else {
    segments[moveseg+1]->setPen(pen);
  }
  reallymoving = true;
}  

void SceneConnectionData::updateMove(QPointF delta) {
  QPointF p0 = moveseg - 1 < 0 ? origpath[0]
                             : origpath[moveseg - 1];
  QPointF p1 = origpath[moveseg] + delta;
  QPointF p2 = origpath[moveseg + 1] + delta;
  QPointF p3 = moveseg + 2  >= origpath.size()
    ? origpath.last() : origpath[moveseg + 2];
  segments[moveseg]->setLine(QLineF(p1, p2));
  SCSegment *seg0 = moveseg - 1 < 0 ? moveseg0
                                  : segments[moveseg - 1];
  if (seg0)
    seg0->setLine(QLineF(p0, p1));
  SCSegment *segN = moveseg + 2 >= origpath.size() ? movesegN
    : segments[moveseg + 1];
  if (segN)
    segN->setLine(QLineF(p2, p3));
}

void SceneConnection::mouseMoveEvent(QGraphicsSceneMouseEvent *e) {
  qDebug() << "SceneConnection::mouseMoveEvent";
  if (d->moveseg<0)
    return;

  QPointF delta = d->moveDelta(e->scenePos(),
			       e->modifiers() & Qt::ControlModifier);
  
  if (!d->reallymoving && !delta.isNull())
    d->startRealMove();
  
  if (d->reallymoving) {
    d->updateMove(delta);
    hover(d->moveseg); // ? or not, this is a choice to be made
  }
}

void SceneConnection::mouseReleaseEvent(QGraphicsSceneMouseEvent *e) {
  qDebug() << "SceneConnection::mouseReleaseEvent";
  if (d->moveseg < 0)
    return;

  if (d->moveseg0)
    delete d->moveseg0;
  d->moveseg0 = 0;
  if (d->movesegN)
    delete d->movesegN;
  d->movesegN = 0;

  if (!d->reallymoving)
    return; // easy, nothing happened

  QPolygonF path = d->origpath;
  setPath(path);
  
  auto const &lib = d->scene->library();
  auto const &circ = d->scene->circuit();
  QPointF delta = d->moveDelta(e->scenePos(),
			       e->modifiers() & Qt::ControlModifier);
  path[d->moveseg] += delta;
  path[d->moveseg+1] += delta;
  if (d->moveseg==0
      && !circ.connection(d->id).danglingStart())
    path.prepend(d->origpath.first());
  if (d->moveseg==d->segments.size()-1
      && !circ.connection(d->id).danglingEnd())
    path.append(d->origpath.last());
  d->scene->modifyConnection(d->id, lib.simplifyPath(path));
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
