// ComponentView.cpp

#include "ComponentView.h"
#include "ORenderer.h"
#include <QPainter>
#include <QPaintEvent>
#include <QDrag>
#include <QMimeData>
#include "ElementView.h"

ComponentView::ComponentView(QWidget *parent): QWidget(parent) {
  id_ = idgen();
  rot = 0;
  flp = false;
  mil2px = 0.2;
  setAcceptDrops(true);
  setPalette(QPalette(QColor(0,0,0)));
}

int ComponentView::idgen() {
  static int lastid = 0;
  return ++lastid;
}

ComponentView::~ComponentView() {
}

int ComponentView::rotation() const {
  return rot;
}

bool ComponentView::isFlipped() const {
  return flp;
}

Group const &ComponentView::group() const {
  return grp;
}

double ComponentView::scale() const {
  return mil2px;
}

void ComponentView::setGroup(class Group const &g) {
  grp = g;
  updateGeometry();
  update();
  emit changed();
}

void ComponentView::setRotation(int r) {
  rot = r&3;
  updateGeometry();
  update();
  emit changed();
}

void ComponentView::setFlipped(bool f) {
  flp = f;
  updateGeometry();
  update();
  emit changed();
}

void ComponentView::setScale(double pxPerMil) {
  mil2px = pxPerMil;
  updateGeometry();
  update();
}


QSize ComponentView::sizeHint() const {
  QSizeF mil = grp.boundingRect().toMils().size();
  if (mil.width() < 300)
    mil.setWidth(300);
  if (mil.height() < 300)
    mil.setHeight(300);
  mil.setWidth(mil.width() + 50);
  mil.setHeight(mil.height() + 50);
  QSizeF px = mil2px * mil;
  if (rot & 1)
    px = QSizeF(px.height(), px.width());
  return px.toSize();
}

QSize ComponentView::minimumSizeHint() const {
  QSizeF mil = grp.boundingRect().toMils().size();
  QSizeF px = mil2px * mil;
  if (rot & 1)
    px = QSizeF(px.height(), px.width());
  return px.toSize();
}


void ComponentView::keyPressEvent(QKeyEvent *e) {
  switch (e->key()) {
  case Qt::Key_R:
    setRotation(rot + ((e->modifiers() & Qt::ShiftModifier) ? 3 : 1));
    emit edited();
    break;
  case Qt::Key_F:
    setFlipped(!flp);
    emit edited();
    break;
  default:
    break;
  }
}

void ComponentView::mousePressEvent(QMouseEvent *e) {
  presspt = e->pos();
  e->accept();
}

void ComponentView::mouseMoveEvent(QMouseEvent *e) {
  e->accept();
  if (!(e->buttons() & Qt::LeftButton))
    return;
  if ((e->pos()-presspt).manhattanLength()<5)
    return;
  
  QDrag *drag = new QDrag(this);
  QMimeData *mimeData = new QMimeData;
  mimeData->setData(dndformat, QString::number(id_).toUtf8());
  drag->setMimeData(mimeData);
  drag->setPixmap(draggable());
  drag->exec(Qt::CopyAction, Qt::CopyAction);
}

void ComponentView::mouseReleaseEvent(QMouseEvent *e) {
  e->accept();
}

void ComponentView::dragEnterEvent(QDragEnterEvent *e) {
  QMimeData const *md = e->mimeData();
  if (md->hasUrls()) {
    QList<QUrl> urls = md->urls();
    QString fn;
    for (QUrl url: urls) {
      if (url.isLocalFile() && url.path().endsWith(".svg")) {
	fn = url.path();
	break;
      }
    }
    if (!fn.isEmpty()) {
      e->accept();
    } else {
      e->ignore();
    }
  } else if (md->hasFormat(dndformat)) {
    e->accept();
  } else {
    e->ignore();
  }
}

void ComponentView::dragMoveEvent(QDragMoveEvent *e) {
  e->accept();
}

void ComponentView::dragLeaveEvent(QDragLeaveEvent *e) {
  e->accept();
}

void ComponentView::dropEvent(QDropEvent *e) {
  QMimeData const *md = e->mimeData();
  if (md->hasUrls()) {
    QList<QUrl> urls = md->urls();
    QString fn;
    for (QUrl url: urls) {
      if (url.isLocalFile() && url.path().endsWith(".svg")) {
	fn = url.path();
	break;
      }
    }
    Group g0;
    int gid = g0.insertComponent(fn);
    if (gid) {
      setGroup(g0.object(gid).asGroup());
      emit edited();
      e->accept();
    } else {
      e->ignore();
    }
  } else if (md->hasFormat(dndformat)) {
    int id = QString(md->data(dndformat)).toInt();
    ElementView const *src = ElementView::instance(id);
    if (src) {
      rot = src->rotation();
      flp = src->isFlipped();
      setGroup(src->group());
      emit edited();
      e->accept();
    } else {
      e->ignore();
    }
  } else {
    e->ignore();
  }
}

QPixmap ComponentView::draggable() const {
  QRectF r = grp.boundingRect().toMils();
  QPointF c = r.center();
  QSizeF s = r.size();
  constexpr int margin = 2;
  double w = (rot&1) ? s.height() : s.width();
  double h = (rot&1) ? s.width() : s.height();
  QPixmap drg(w*mil2px + 2*margin, h*mil2px + 2*margin);
  drg.fill(QColor(0,0,0,0));
  QPainter p(&drg);
  p.translate(drg.width()/2, drg.height()/2);
  if (rot)
    p.rotate(rot*90*16); // is that right?
  if (flp)
    p.scale(-1, 1);
  p.scale(mil2px, mil2px);
  p.translate(-c.x(), -c.y());
  ORenderer::render(grp, &p);
  return drg;
}

void ComponentView::paintEvent(QPaintEvent *) {
  QPainter p(this);
  p.setPen(QPen(Qt::NoPen));
  p.setBrush(QBrush(QColor(0,0,0)));
  p.drawRect(QRect(QPoint(0,0), size()));
  p.translate(width()/2, height()/2);
  if (rot)
    p.rotate(rot*90*16); // is that right?
  if (flp)
    p.scale(-1, 1);
  p.scale(mil2px, mil2px);
  QRectF r = grp.boundingRect().toMils();
  QPointF c = r.center();
  p.translate(-c.x(), -c.y());
  ORenderer::render(grp, &p);
}

int ComponentView::id() const {
  return id_;
}
