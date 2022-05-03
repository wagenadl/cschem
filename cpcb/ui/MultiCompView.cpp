// MultiCompView.cpp

#include "MultiCompView.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "ElementView.h"
#include "data/Group.h"
#include "data/Object.h"
#include "data/Paths.h"
#include "circuit/Schem.h"
#include "svg/Symbol.h"
#include "svg/SymbolLibrary.h"
#include <QRegularExpression>
#include <QDir>
#include <QScrollBar>

class MCVData {
public:
  MCVData(MultiCompView *mcv): mcv(mcv) {
    lay = new QVBoxLayout;
    lay->setContentsMargins(4, 4, 4, 4);
    lay->setSpacing(4);
    lay->addStretch(1);
    mcv->setWidget(new QWidget);
    mcv->setWidgetResizable(true);
    mcv->widget()->setLayout(lay);
    mcv->setMinimumWidth(100);
    ppm = 0.2;
    ed = 0;
  }
  void rebuild();
  void perhapsSaveDefault(QString);
  void setMinWidth();
public:
  MultiCompView *mcv;
  Schem schem;
  Group root;
  QMap<QString, ElementView *> evs; // maps ref to EV
  QBoxLayout *lay;
  double ppm;
  class Editor *ed;
};

static QString simplifiedSymbol(QString pv) {
  pv.replace("part:", "");
  pv.replace("container:", "");
  pv.replace("diode:", "");
  pv.replace("transistor:", "");
  return pv;
}

static QString cleansedFilename(QString fn) {
  QRegularExpression re("[^A-Za-z0-9]");
  fn.replace(re, "-");
  return fn;
}

void MCVData::perhapsSaveDefault(QString ref) {
  QString sym;
  for (Element const &elt: schem.circuit().elements)
    if (elt.name==ref)
      if (sym.isEmpty() || elt.isContainer())
	sym = elt.symbol();
  if (evs.contains(ref)) {
    Group const &grp = evs[ref]->group();
    if (!grp.isEmpty()) {
      Group root;
      int id = root.insert(Object(grp));
      QDir::root().mkpath(Paths::recentSymbolsLocation());
      root.saveComponent(id, Paths::recentSymbolsLocation() + "/"
			 + cleansedFilename(sym) + ".svg");
    }
  }
}

void MCVData::setMinWidth() {
  int mw = 100;
  for (auto *ev: evs) {
    int w1 = ev->sizeHint().width();
    if (w1>mw)
      mw = w1;
  }
  auto *sb = mcv->verticalScrollBar();
  if (sb)
    mw += sb->width();
  mw += 10; // a little extra margin
  mcv->setMinimumWidth(mw);
}

static QString containerFor(QString ref) {
  int dotidx = ref.indexOf(".");
  if (dotidx>0)
    return ref.left(dotidx);
  else
    return "---";
}

void MCVData::rebuild() {
  QMap<QString, Element> newelts;
  for (Element const &elt: schem.circuit().elements)
    if (elt.type==Element::Type::Component)
      newelts[elt.name] = elt;
  QSet<QString> newused; // i.e., already in pcb layout
  for (int k: root.keys()) {
    Object const &obj(root.object(k));
    if (obj.isGroup())
      newused << obj.asGroup().ref;
  }
  //qDebug() << "newused" << newused;
  //qDebug() << "newelts" << newelts.keys();
  for (QString ref: evs.keys()) {
    if (!newelts.contains(ref)
	|| newused.contains(ref)
	|| newused.contains(containerFor(ref))) {
      delete evs[ref];
      evs.remove(ref);
    }
  }
  newelts.remove("");
  for (QString ref: newelts.keys()) {
    if (!newused.contains(ref) && !newused.contains(containerFor(ref))) {
      bool trulynew = !evs.contains(ref);
      if (trulynew) {
	evs[ref] = new ElementView;
        evs[ref]->setScale(ppm);
        evs[ref]->linkEditor(ed);
	int idx = evs.keys().indexOf(ref);
	lay->insertWidget(idx, evs[ref]);
	QObject::connect(evs[ref], &ElementView::changed,
			 [this, ref]() { perhapsSaveDefault(ref); });
      }
      evs[ref]->setRefText(ref);
      QString pv = newelts[ref].value;
      if (pv.isEmpty())
	pv = "<i>" + simplifiedSymbol(newelts[ref].symbol()) + "</i>";
      evs[ref]->setPVText(pv);
      QString sym = newelts[ref].symbol();
      Symbol const &symbol(schem.library().symbol(sym));
      if (symbol.isValid()) {
	int npins = symbol.totalPinCount();
	QString pin = npins==1 ? "pin" : "pins";
	evs[ref]->setPinCount(npins);
	evs[ref]->setFallbackText("(" + QString::number(npins)
				  + " " + pin + ")");
      } else {
	evs[ref]->setFallbackText("??");
      }
      if (trulynew) {
	QDir dir(Paths::recentSymbolsLocation());
	QString fn = cleansedFilename(sym) + ".svg";
	if (dir.exists(fn)) {
	  Group root;
	  int id = root.insertComponent(dir.absoluteFilePath(fn));
	  if (id>0)
	    evs[ref]->setGroup(root.object(id).asGroup());
	}
      }
    }
  }
  setMinWidth();
}

MultiCompView::MultiCompView(QWidget *parent):
  QScrollArea(parent), d(new MCVData(this)) {
}

MultiCompView::~MultiCompView() {
  delete d;
}

Schem const &MultiCompView::schem() const {
  return d->schem;
}

Group const &MultiCompView::root() const {
  return d->root;
}

void MultiCompView::setSchem(Schem const &s) {
  d->schem = s;
  d->rebuild();
}

void MultiCompView::setRoot(Group const &g) {
  d->root = g;
  d->rebuild();
}

void MultiCompView::setScale(double pxPerMil) {
  for (auto ev: d->evs)
    ev->setScale(pxPerMil);
  d->ppm = pxPerMil;
  d->setMinWidth();
}


void MultiCompView::linkEditor(class Editor *ed) {
  for (auto ev: d->evs)
    ev->linkEditor(ed);
  d->ed = ed;
}
  
