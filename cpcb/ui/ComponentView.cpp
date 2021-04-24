// ComponentView.cpp

#include "ComponentView.h"
#include "ORenderer.h"
#include <QPainter>
#include <QPaintEvent>
#include <QDrag>
#include <QMimeData>
#include "ElementView.h"
#include  <QFontMetricsF>

ComponentView::ComponentView(QWidget *parent): QWidget(parent) {
  id_ = idgen();
  mil2px = 0.2;
  npins = 0;
  setAcceptDrops(true);
  setFocusPolicy(Qt::StrongFocus);
  setPalette(QPalette(QColor(0,0,0)));
  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}

int ComponentView::idgen() {
  static int lastid = 0;
  return ++lastid;
}

ComponentView::~ComponentView() {
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

void ComponentView::rotateCW() {
  grp.rotateCW(grp.anchor());
  updateGeometry();
  update();
  emit changed();
}

void ComponentView::rotateCCW() {
  grp.rotateCCW(grp.anchor());
  updateGeometry();
  update();
  emit changed();
}

void ComponentView::flipLeftRight() {
  grp.flipLeftRight(grp.anchor().x);
  updateGeometry();
  update();
  emit changed();
}

void ComponentView::flipUpDown() {
  grp.flipUpDown(grp.anchor().y);
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
  if (mil.height() < 200)
    mil.setHeight(200);
  mil.setWidth(mil.width() + 50);
  mil.setHeight(mil.height() + 50);
  QSizeF px = mil2px * mil;
  QFontMetricsF fm(font());
  QSizeF fs(fm.boundingRect("XXXXXXXX").size());
  if (px.width() < fs.width())
    px.setWidth(fs.width());
  if (px.height() < 3.4*fs.height())
    px.setHeight(3.4*fs.height());
  return px.toSize();
}

QSize ComponentView::minimumSizeHint() const {
  QSizeF mil = grp.boundingRect().toMils().size();
  QSizeF px = mil2px * mil;
  return px.toSize();
}


void ComponentView::keyPressEvent(QKeyEvent *e) {
  switch (e->key()) {
  case Qt::Key_R:
    if (e->modifiers() & Qt::ShiftModifier)
      rotateCCW();
    else
      rotateCW();
    emit edited();
    break;
  case Qt::Key_F:
    if (e->modifiers() & Qt::ShiftModifier)
      flipUpDown();
    else
      flipLeftRight();
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
  if (group().isEmpty())
    return;
  
  QDrag *drag = new QDrag(this);
  QMimeData *mimeData = new QMimeData;
  mimeData->setData(dndformat, QString::number(id_).toUtf8());
  drag->setMimeData(mimeData);
  QPixmap drg(draggable());
  drag->setPixmap(drg);
#if 0
  drag->setHotSpot(mapWidgetToDraggable(drg, presspt));
#else
  QStringList pn(grp.pinNames());
  if (pn.isEmpty())
    drag->setHotSpot(mapGroupToDraggable(drg, grp.boundingRect().center()));
  else
    drag->setHotSpot(mapGroupToDraggable(drg, grp.pinPosition(pn.first())));
#endif
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
      if (url.isLocalFile()) {
        QString fn1 = url.toLocalFile();
        if (fn1.toLower().endsWith(".svg")) {
          fn = fn1;
          break;
        }
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
      if (url.isLocalFile()) {
        QString fn1 = url.toLocalFile();
        if (fn1.toLower().endsWith(".svg")) {
          fn = fn1;
          break;
        }
      }
    }
    Group g0;
    int gid = g0.insertComponent(fn);
    if (gid) {
      Group const &g1(g0.object(gid).asGroup());
      if (npins==0 || g1.pinNames().size()==npins) {
	setGroup(g1);
	emit edited();
	e->accept();
	return;
      }
    }
  } else if (md->hasFormat(dndformat)) {
    int id = QString(md->data(dndformat)).toInt();
    ElementView const *src = ElementView::instance(id);
    if (src) {
      Group const &g1 = src->group();
      if (npins==0 || g1.pinNames().size()==npins) {
	setGroup(src->group());
	emit edited();
	e->accept();
	return;
      }
    }
  }
  e->ignore();
}

QPoint ComponentView::mapWidgetToDraggable(QPixmap const &drg,
					   QPoint pw) const {
  QPoint cw(width()/2, height()/2);
  QPoint cd(drg.width()/2, drg.height()/2);
  return pw - cw + cd;
}

QPoint ComponentView::mapGroupToDraggable(QPixmap const &drg, Point p) const {
  QPointF pg = (p - grp.boundingRect().center()).toMils();
  // in mils relative to center of bounding rect
  return QPoint(drg.width()/2, drg.height()/2) + (pg*mil2px).toPoint();
}

QPixmap ComponentView::draggable() const {
  QRectF r = grp.boundingRect().toMils();
  QPointF c = r.center();
  QSizeF s = r.size();
  constexpr int margin = 2;
  QPixmap drg(s.width()*mil2px + 2*margin, s.height()*mil2px + 2*margin);
  drg.fill(QColor(0,0,0,0));
  QPainter p(&drg);
  p.translate(drg.width()/2, drg.height()/2);
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
  if (grp.isEmpty()) {
    p.setPen(QPen(QColor(255,255,255)));
    p.drawText(QRect(QPoint(0,0), size()), Qt::AlignCenter, fbtxt);
  } else {
      p.translate(width()/2, height()/2);
    p.scale(mil2px, mil2px);
    QRectF r = grp.boundingRect().toMils();
    QPointF c = r.center();
    p.translate(-c.x(), -c.y());
    ORenderer::render(grp, &p);
  }
}

int ComponentView::id() const {
  return id_;
}

void ComponentView::setFallbackText(QString s) {
  fbtxt = s;
  update();
}

void ComponentView::setPinCount(int n) {
  npins = n;
}
