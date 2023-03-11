// LibView.cpp

#include "LibView.h"
#include <QDebug>
#include "ui/SvgItem.h"
#include "svg/SymbolLibrary.h"
#include <QGraphicsSceneMouseEvent>
#include <QDrag>
#include <QMimeData>
#include <QGraphicsColorizeEffect>
#include "Style.h"

class LibViewElement: public SvgItem {
public:
  LibViewElement(QString name, QString pop, LibView *view):
    name(name), pop(pop), view(view) {
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
  QString pop;
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
  void addHeader(QString symbol, QString label);
  double nextY() const;
public:
  LibView *view;
  QGraphicsScene *scene;
  SymbolLibrary const *lib;
  QMap<QString, SvgItem *> items;
  QMap<QString, QGraphicsTextItem *> headers;
};

void LibViewElement::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *) {
  view->activate(name, pop);
}

void LibViewElement::mouseMoveEvent(QGraphicsSceneMouseEvent *e) {
  QPointF p1 = e->scenePos();
  if (!drg && (p1-p0).manhattanLength() >= 5) {
    drg = true;
    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;
    mimeData->setData("application/x-dnd-cschem",
                      (name + "::" + pop).toUtf8());
    drag->setMimeData(mimeData);
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
  view->hover(name, pop);
}
  
void LibViewElement::hoverLeaveEvent(QGraphicsSceneHoverEvent *) {
  setGraphicsEffect(0);
  view->unhover();
}

double LibViewData::nextY() const {
  return (items.isEmpty() && headers.isEmpty())
	  ? 0 : scene->itemsBoundingRect().bottom() + 14;
}
    
void LibViewData::addSymbol(QString symbol) {
  Symbol const &smb = lib->symbol(symbol);
  SvgItem *item = new LibViewElement(symbol, smb.popupName(), view);
  item->setRenderer(smb.renderer());
  scene->addItem(item);
  items[symbol] = item;
  item->setPos(QPointF(0, nextY()));
}

void LibViewData::addHeader(QString symbol, QString label) {
  QGraphicsTextItem *header = new QGraphicsTextItem(label);
  QFont f = header->font();
  f.setStyle(QFont::StyleItalic);
  f.setPointSize(f.pointSize() * .75);
  header->setFont(f);
  scene->addItem(header);
  headers[symbol] = header;
  header->setPos(QPointF(0, nextY()));
}

LibView::LibView(QWidget *parent): QGraphicsView(parent),
                                   d(new LibViewData(this)) {
  setScene(d->scene);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  resize(92, 1000);
}

void LibView::clear() {
  for (auto *i: d->items)
    delete i;
  d->items.clear();
  for (auto *i: d->headers)
    delete i;
  d->headers.clear();
  d->scene->setSceneRect(d->scene->itemsBoundingRect());
}

static bool symbolLessThan(QString a, QString b) {
  int portdif = b.startsWith("port:") - a.startsWith("port:");
  int contdif = b.contains(":container:") - a.contains(":container:");
  if  (portdif)
    return portdif<0;
  if (contdif)
    return contdif>0;
  return a.toLower() < b.toLower();
}

void  LibView::setLibrary(class SymbolLibrary const *lib) {
  d->lib = lib;
  rebuild();
}

void LibView::rebuild() {
  clear();
   
  if (!d->lib)
    return;

  QStringList symbols = d->lib->symbolNames();
  std::sort(symbols.begin(), symbols.end(),
	    [](QString a, QString b) { return symbolLessThan(a,b); });
  QString lastheader = "";
  for (QString s: symbols) {
    if (!(s.startsWith("port:") || s.startsWith("part:")))
      continue;
    QString header;
    if (s.startsWith("port:"))
      header = "Ports";
    else if (s.contains(":container:"))
      header = "Containers";
    else if (s.contains(":battery"))
      header = "Battery";
    else if (s.contains(":connector"))
      header = "Connectors";
    else if (s.contains(":jumper"))
      header = "Jumpers";
    else if (s.contains(":diode"))
      header = "Diodes";
    else if (s.contains(":ic"))
      header = "ICs";
    else if (s.contains(":logic"))
      header = "Logic";
    else if (s.contains(":passive"))
      header = "Passives";
    else if (s.contains(":switch"))
      header = "Switches";
    else if (s.contains(":transistor"))
      header = "Transistors";
    else
      header = "Parts";
    if (header != lastheader) {
      d->addHeader(s, header);
      lastheader = header;
    }
    d->addSymbol(s);
  }
  
  QRectF r = d->scene->itemsBoundingRect();
  
  double w = r.width();
  for (auto *it: d->items) {
    QPointF p = it->pos();
    QRectF bb = it->sceneBoundingRect();
    it->setPos(QPointF(w/2 - bb.width()/2, p.y()));
  }

  d->scene->setSceneRect(r.adjusted(-7, -14, 7, 14));
  setMinimumWidth(92);
}

void LibView::resizeEvent(QResizeEvent *e) {
  QGraphicsView::resizeEvent(e);
  double scl = transform().m11();
  double x = viewport()->width() / sceneRect().width() / scl;
  QGraphicsView::scale(x, x);
}

void LibView::scale(double x) {
  return;
  QGraphicsView::scale(x, x);
  QRectF r = d->scene->sceneRect();
  setMinimumWidth(mapFromScene(r.bottomRight()).x()
		  - mapFromScene(r.topLeft()).x());
}

LibView::~LibView() {
  delete d;
}

void LibView::activate(QString symbol, QString pop) {
  qDebug () <<"libview act" << symbol << pop;
  emit activated(symbol, pop);
}

void LibView::hover(QString symbol, QString pop) {
  qDebug () <<"libview hover" << symbol << pop;
  emit hoveron(symbol, pop);
}

void LibView::unhover() {
  qDebug () <<"libview unhover";
  emit hoveroff();
}
