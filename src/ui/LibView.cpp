// LibView.cpp

#include "LibView.h"
#include <QDebug>
#include <QGraphicsSvgItem>
#include "svg/PartLibrary.h"
#include <QGraphicsSceneMouseEvent>
#include <QDrag>
#include <QMimeData>

class LibViewElement: public QGraphicsSvgItem {
public:
  LibViewElement(QString name, LibView *view): name(name), view(view) { }
  ~LibViewElement() { }
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *) override;
  void mouseMoveEvent(QGraphicsSceneMouseEvent *) override;
  void mousePressEvent(QGraphicsSceneMouseEvent *) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *) override;
private:
  QString name;
  LibView *view;
  QPointF p0;
  bool drg;
};

class LibViewData {
public:
  LibViewData(LibView *view): view(view), scene(new QGraphicsScene) {
  }
  ~LibViewData() {
    delete scene; // that deletes the items, right?
  }
  void addPart(PartLibrary const *lib, QString part);
public:
  LibView *view;
  QGraphicsScene *scene;
  QMap<QString, QGraphicsSvgItem *> items;
};

void LibViewElement::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *) {
  view->activate(name);
}

void LibViewElement::mouseMoveEvent(QGraphicsSceneMouseEvent *e) {
  QPointF p1 = e->scenePos();
  if (!drg && (p1-p0).manhattanLength() >= 5) {
    drg = true;
    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;
    mimeData->setData("application/x-dnd-cschem", name.toUtf8());
    drag->setMimeData(mimeData);
    qDebug() << "Drag started for" << name;
    drag->exec(Qt::CopyAction, Qt::CopyAction);
  }
}

void LibViewElement::mousePressEvent(QGraphicsSceneMouseEvent *e) {
  p0 = e->scenePos();
  drg = false;
  e->accept();
}

void LibViewElement::mouseReleaseEvent(QGraphicsSceneMouseEvent *) {
}



void LibViewData::addPart(PartLibrary const *lib, QString part) {
  double y = items.isEmpty() ? 0 : scene->sceneRect().bottom() + 14;
  QGraphicsSvgItem *item = new LibViewElement(part, view);
  item->setSharedRenderer(lib->renderer(part).data());
  scene->addItem(item);
  items[part] = item;
  item->setPos(QPointF(0, y));
}

LibView::LibView(QWidget *parent): QGraphicsView(parent),
                                   d(new LibViewData(this)) {
  setScene(d->scene);
}

LibView::LibView(class PartLibrary const *lib, QWidget *parent):
  LibView(parent) {
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  rebuild(lib);
}
void LibView::rebuild(class PartLibrary const *lib) {
  for (auto *i: d->items)
    delete i;
  d->items.clear();

  QStringList parts = lib->partNames();
  std::sort(parts.begin(), parts.end());
  for (QString s: parts) 
    if (s.startsWith("port:") || s.startsWith("part:"))
      d->addPart(lib, s);

  QRectF r = d->scene->itemsBoundingRect();
  
  double w = r.width();
  for (auto *it: d->items) {
    QPointF p = it->pos();
    QRectF bb = it->sceneBoundingRect();
    it->setPos(QPointF(w/2 - bb.width()/2, p.y()));
  }

  d->scene->setSceneRect(r.adjusted(-7, -14, 7, 14));
}

LibView::~LibView() {
  delete d;
}

void LibView::activate(QString part) {
  qDebug() << "LibView::activate" << part;
  emit activated(part);
}
