// MultiCompView.cpp

#include "MultiCompView.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "ElementView.h"
#include "data/Group.h"
#include "data/Object.h"
#include "circuit/Schem.h"
#include "svg/Symbol.h"
#include "svg/SymbolLibrary.h"

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
    mcv->setMinimumWidth(200);
  }
  void rebuild();
public:
  MultiCompView *mcv;
  Schem schem;
  Group root;
  QMap<QString, ElementView *> evs; // maps ref to EV
  QBoxLayout *lay;
};

void MCVData::rebuild() {
  QMap<QString, Element> newelts;
  for (Element const &elt: schem.circuit().elements)
    if (elt.type==Element::Type::Component)
      newelts[elt.name] = elt;
  QSet<QString> newused;
  for (int k: root.keys()) {
    Object const &obj(root.object(k));
    if (obj.isGroup())
      newused << obj.asGroup().ref;
  }
  for (QString ref: evs.keys()) {
    if (!newelts.contains(ref) || newused.contains(ref)) {
      delete evs[ref];
      evs.remove(ref);
    }
  }
  newelts.remove("");
  qDebug() << "newelts" << newelts;
  for (QString ref: newelts.keys()) {
    if (!newused.contains(ref)) {
      if (!evs.contains(ref)) {
	evs[ref] = new ElementView;
	int idx = evs.keys().indexOf(ref);
	lay->insertWidget(idx, evs[ref]);
      }
      evs[ref]->setRefText(ref);
      QString pv = newelts[ref].value;
      if (pv.isEmpty())
	pv = "<i>" + newelts[ref].symbol() + "</i>";
      pv.replace("part:", "");
      evs[ref]->setPVText(pv);
      Symbol const &symbol(schem.library().symbol(newelts[ref].symbol()));
      if (symbol.isValid()) {
	int npins = symbol.pinNames().size();
	for (int n: symbol.containerSlots())
	  npins += symbol.containedPins(n).size();
	QString pin = npins==1 ? "pin" : "pins";
	evs[ref]->setFallbackText("(" + QString::number(npins)
				  + " " + pin + ")");
      } else {
	evs[ref]->setFallbackText("??");
      }
      qDebug() <<"now, set default package";
    }
  }
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

