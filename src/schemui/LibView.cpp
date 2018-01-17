// LibView.cpp

#include "LibView.h"
#include <QDebug>
#include "qt/SvgItem.h"
#include "svg/SymbolLibrary.h"
#include <QGraphicsSceneMouseEvent>
#include <QDrag>
#include <QMimeData>
#include <QGraphicsColorizeEffect>
#include "Style.h"

class LibViewElement: public SvgItem {
public:
  LibViewElement(QString name, LibView *view): name(name), view(view) {
    setAcceptHoverEvents(true);
  }
  ~LibViewElement() { }
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *) override;
  void mouseMoveEvent(QGraphicsSceneMouseEvent *) override;
  void mousePressEvent(QGraphicsSceneMouseEvent *) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *) override;
  void hoverEnterEvent(QGraphicsSceneHoverEvent *) override;
  void hoverLeaveEvent(QGraphicsSceneHoverEvent *) override;
private:
  QString name;
  LibView *view;
  QPointF p0;
  bool drg;
};

class LibViewData {
public:
  LibViewData(LibView *view): view(view), scene(new QGraphicsScene), lib(0) {
  }
  ~LibViewData() {
    delete scene; // that deletes the items, right?
  }
  void addSymbol(QString symbol);
public:
  LibView *view;
  QGraphicsScene *scene;
  SymbolLibrary const *lib;
  QMap<QString, SvgItem *> items;
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
    if (drag->exec(Qt::CopyAction, Qt::CopyAction))
      setGraphicsEffect(0);
  }
}

void LibViewElement::mousePressEvent(QGraphicsSceneMouseEvent *e) {
  p0 = e->scenePos();
  drg = false;
  e->accept();
}

void LibViewElement::mouseReleaseEvent(QGraphicsSceneMouseEvent *e) {
  e->accept();
}

void LibViewElement::hoverEnterEvent(QGraphicsSceneHoverEvent *) {
  auto *ef = new QGraphicsColorizeEffect;
  ef->setColor(Style::hoverColor());
  setGraphicsEffect(ef);
}
  
void LibViewElement::hoverLeaveEvent(QGraphicsSceneHoverEvent *) {
  setGraphicsEffect(0);
}

void LibViewData::addSymbol(QString symbol) {
  double y = items.isEmpty() ? 0 : scene->itemsBoundingRect().bottom() + 14;
  SvgItem *item = new LibViewElement(symbol, view);
  item->setRenderer(lib->symbol(symbol).renderer());
  scene->addItem(item);
  items[symbol] = item;
  item->setPos(QPointF(0, y));
}

LibView::LibView(QWidget *parent): QGraphicsView(parent),
                                   d(new LibViewData(this)) {
  setScene(d->scene);
}

LibView::LibView(class SymbolLibrary const *lib, QWidget *parent):
  LibView(parent) {
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  rebuild(lib);
}

void LibView::clear() {
  for (auto *i: d->items)
    delete i;
  d->items.clear();
  d->scene->setSceneRect(d->scene->itemsBoundingRect());
}

void LibView::rebuild(class SymbolLibrary const *lib) {
  if (lib)
    d->lib = lib;

  clear();
   
  if (!d->lib)
    return;

  QStringList symbols = d->lib->symbolNames();
  std::sort(symbols.begin(), symbols.end(),
	    [](QString a, QString b) { return a.toLower() < b.toLower(); });
  qDebug() << "symbolnames" << symbols;
  for (QString s: symbols) 
    if (s.startsWith("port:") || s.startsWith("part:"))
      d->addSymbol(s);

  QRectF r = d->scene->itemsBoundingRect();
  
  double w = r.width();
  for (auto *it: d->items) {
    QPointF p = it->pos();
    QRectF bb = it->sceneBoundingRect();
    it->setPos(QPointF(w/2 - bb.width()/2, p.y()));
  }

  d->scene->setSceneRect(r.adjusted(-7, -14, 7, 14));
  scale(1);
}

void LibView::scale(double x) {
  QGraphicsView::scale(x, x);
  QRectF r = d->scene->sceneRect();
  setMinimumWidth(mapFromScene(r.bottomRight()).x()
		  - mapFromScene(r.topLeft()).x());
}

LibView::~LibView() {
  delete d;
}

void LibView::activate(QString symbol) {
  qDebug() << "LibView::activate" << symbol;
  emit activated(symbol);
}
