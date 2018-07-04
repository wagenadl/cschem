// MultiCompView.cpp

#include "MultiCompView.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "ElementView.h"
#include "data/Group.h"
#include "data/Object.h"
#include "circuit/Schem.h"

class MCVData {
public:
  MCVData(MultiCompView *mcv): mcv(mcv) {
    lay = new QVBoxLayout;
    mcv->setLayout(lay);
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
  for (QString ref: newelts.keys()) {
    if (!newused.contains(ref)) {
      if (!evs.contains(ref)) {
	evs[ref] = new ElementView;
	int idx = evs.keys().indexOf(ref);
	lay->insertWidget(idx, evs[ref]);
      }
      evs[ref]->setRefText(ref);
      evs[ref]->setPVText(newelts[ref].value);
      // now, set default package?
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

