// LinkedSchematic.cpp

#include "LinkedSchematic.h"
#include "../../cschem/src/circuit/Schem.h"
#include "../../cschem/src/file/FileIO.h"

#include <QFileSystemWatcher>

class LSData {
public:
  LSData(LinkedSchematic *ls) {
    watcher = new QFileSystemWatcher(ls);
  }
  void invalidateNets();
  void validateNets();
public:
  QString fn;
  Schem schem;
  QFileSystemWatcher *watcher;
  mutable QList<Net> nets;
  mutable bool havenets;
};

void LSData::invalidateNets() {
  havenets = false;
  nets.clear();
}

void LSData::validateNets() {
  if (havenets)
    return;
  nets = Net::allNets(schem.circuit());
  havenets = true;
}

LinkedSchematic::LinkedSchematic(QObject *parent):
  QObject(parent), d(new LSData(this)) {
  connect(d->watcher, &QFileSystemWatcher::fileChanged,
	  [this](QString) {
	    qDebug() << "Reloading linked schematic";
	    d->invalidateNets();
	    d->schem = FileIO::loadSchematic(d->fn);
	    reloaded();
	  });
}


void LinkedSchematic::link(QString fn) {
  unlink();
  if (fn.isEmpty())
    return;
  d->fn = fn;
  d->watcher->addPath(d->fn);
  d->schem = FileIO::loadSchematic(fn);
}

void LinkedSchematic::unlink() {
  d->invalidateNets();
  if (!d->fn.isEmpty())
    d->watcher->removePath(d->fn);
  d->fn = "";
  d->schem = Schem();
}

LinkedSchematic::~LinkedSchematic() {
  delete d;
}

bool LinkedSchematic::isValid() const {
  return !d->fn.isEmpty();
}

Schem LinkedSchematic::schematic() const {
  return d->schem;
}

Circuit LinkedSchematic::circuit() const {
  return d->schem.circuit();
}

QList<Net> LinkedSchematic::nets() const {
  d->validateNets();
  return d->nets;
}
