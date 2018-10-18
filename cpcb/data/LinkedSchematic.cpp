// LinkedSchematic.cpp

#include "LinkedSchematic.h"
#include "circuit/Schem.h"
#include "file/FileIO.h"

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
  mutable QList<LinkedNet> nets;
  mutable bool havenets;
  mutable QMap<Nodename, Nodename> aliases;
};

void LSData::invalidateNets() {
  havenets = false;
  nets.clear();
}

static QString netalias(QString key, QMap<QString, QString> const &map) {
  if (map.contains(key)) {
    QString val = map[key];
    if (val!=key)
      return netalias(val, map);
    else
      return val;
  } else {
    return key;
  }
}

void LSData::validateNets() {
  if (havenets)
    return;
  Circuit const &circ(schem.circuit());
  nets.clear();
  QMap<QString, Net> netmap;
  QMap<QString, QString> netaliases;
  auto addAlias = [&](QString key, QString val) {
    if (key!=val)
      netaliases[key] = val;
  };
  for (Net const &net: Net::allNets(circ)) {
    QString name= net.name();
    if (netmap.contains(name)) {
      netmap[name].merge(net);
      for (QString p: netmap[name].ports())
	addAlias(p, name);
    } else {
      netmap[name] = net;
      for (QString p: net.ports())
	addAlias(p, name);
    }
  }

  // now, combine all aliases
  qDebug() << netaliases;
  for (QString a: netaliases.keys()) {
    QString v = netalias(a, netaliases);
    if (v!=a) {
      netmap[v].merge(netmap[a]);
      netmap.remove(a);
    }
  }
  
  for (Net const &net: netmap)
    nets << LinkedNet(schem, net);

  for (LinkedNet const &lnet: nets) {
    for (Nodename const  &nn: lnet.nodes) {
      if (nn.hasPinNumber() && nn.hasPinName()) {
	QString num(QString::number(nn.pinNumber()));
	QString name(nn.pinName());
	int dotidx = name.indexOf('.');
	QString comp = nn.component();
	QString compa = comp;
	if (dotidx>0) {
	  compa += "." + name.left(dotidx);
	  name = name.mid(dotidx+1);
	}
	aliases[Nodename(comp, num)] = Nodename(compa, name);
      }
    }
  }
  havenets = true;
}

LinkedSchematic::LinkedSchematic(QObject *parent):
  QObject(parent), d(new LSData(this)) {
  connect(d->watcher, &QFileSystemWatcher::fileChanged,
	  [this](QString) {
	    d->invalidateNets();
            Schem s = FileIO::loadSchematic(d->fn);
            if (!s.isEmpty()) {
              d->schem = s;
              qDebug() << "Reloaded linked schematic";
              reloaded();
            }
	  });
}


void LinkedSchematic::link(QString fn) {
  unlink();
  if (fn.isEmpty())
    return;
  d->fn = fn;
  qDebug() << "adding to watched files" << d->fn;
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

QList<LinkedNet> LinkedSchematic::nets() const {
  d->validateNets();
  return d->nets;
}

Nodename LinkedSchematic::pinAlias(Nodename const &nn) const {
  d->validateNets();
  if (d->aliases.contains(nn))
    return d->aliases[nn];
  else
    return Nodename("", "");
}
