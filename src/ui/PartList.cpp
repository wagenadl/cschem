// PartList.cpp

#include "PartList.h"
#include "Scene.h"
#include "circuit/Circuit.h"
#include "circuit/Element.h"
#include "circuit/PartNumbering.h"
#include <QDebug>

class PartListData {
public:
  PartListData(Scene *scene): scene(scene) {
    elements = scene->circuit().elements().values();
  }
  void setName(int row, QString name) {
    if (elements[row].name() == name)
      return;
    elements[row].setName(name);
    scene->updateFromPartList(elements[row]);
  }
  void setValue(int row, QString value) {
    value = PartNumbering::prettyValue(value, elements[row].name());
    if (elements[row].value() == value)
      return;
    elements[row].setValue(value);
    scene->updateFromPartList(elements[row]);
  }    
public:
  Scene *scene;
  QList<Element> elements; // list elements are rows of the model
};

PartList::PartList(class Scene *scene):
  QAbstractTableModel(scene), // scene is our parent from a QObject perspective
  d(new PartListData(scene)) {
}

PartList::~PartList() {
  delete d;
}

QVariant PartList::data(QModelIndex const &index,
                        int role) const {
  int r = index.row();
  int c = index.column();
  bool iseditdisp = role==Qt::EditRole || role==Qt::DisplayRole;
  if (!iseditdisp || r<0 || c<0 || r>=d->elements.size() || c>=int(Column::N))
    return QVariant();
  Element const &elt = d->elements[r];
  switch (Column(c)) {
  case Column::Id: return elt.id();
  case Column::Name: return elt.name();
  case Column::Value: return elt.value();
  case Column::Vendor: return elt.info().vendor;
  case Column::CatNo: return elt.info().partno;
  case Column::Package: return elt.info().package;
  case Column::Notes: return elt.info().notes;
  default: return QVariant();
  }
}

bool PartList::setData(QModelIndex const &index, QVariant const &value,
             int role) {
  int r = index.row();
  int c = index.column();
  bool isedit = role==Qt::EditRole;
  if (!isedit || r<0 || c<0 || r>=d->elements.size() || c>=int(Column::N))
    return false;

  Element &elt(d->elements[r]);
  Element::Info info = elt.info();
  switch (Column(c)) {
  case Column::Name:
    d->setName(r, value.toString());
    break;
  case Column::Value:
    d->setValue(r, value.toString());
    break;
  case Column::Vendor:
    info.vendor = value.toString();
    elt.setInfo(info);
    break;
  case Column::CatNo: 
    info.partno = value.toString();
    elt.setInfo(info);
    break;
  case Column::Package: 
    info.package = value.toString();
    elt.setInfo(info);
    break;
  case Column::Notes: 
    info.notes = value.toString();
    elt.setInfo(info);
    break;
  default:
    return false;
  }
  emit dataChanged(index, index);
  return true;
}

Qt::ItemFlags PartList::flags(QModelIndex const &index) const {
  int c = index.column();
  Qt::ItemFlags f = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  if (Column(c) != Column::Id)
    f |= Qt::ItemIsEditable;
  return f;
}

QVariant PartList::headerData(int section, Qt::Orientation orientation,
                    int role) const {
  if (role != Qt::DisplayRole)
    return QVariant();
  if (orientation==Qt::Horizontal) {
    // working on columns
    switch (Column(section)) {
    case Column::Id:
      return QString("Id");
    case Column::Name:
      return QString("Ref.");
    case Column::Value:
      return QString("Value");
    case Column::Vendor:
      return QString("Vendor");
    case Column::CatNo:
      return QString("Cat.#");
    case Column::Package:
      return QString("Pkg.");
    case Column::Notes:
      return QString("Notes");
    default:
      return QVariant();
    }
  } else {
    // working on rows
    return ""; // this should never be shown
  }
}

int PartList::columnCount(QModelIndex const &) const {
  return int(Column::N);
}

int PartList::rowCount(QModelIndex const &parent) const {
  qDebug() << "rowcount" << d->elements.size();
  if (parent.isValid())
    return 0;
  else
    return d->elements.size();
}

void PartList::rebuild() {
  // Regrab circuit from scene, update rows as needed.
  // Let's first see what we simply need to drop
  QMap<int, Element> newmap = d->scene->circuit().elements();
  for (int n: newmap.keys())
    if (newmap[n].type() != Element::Type::Component)
      newmap.remove(n);
  
  qDebug() << "newmap.size" << newmap.size();
  
  int N = d->elements.size();
  qDebug() << "old.size" << N;
  int n = 0;
  while (n<N) {
    Element const &elt = d->elements[n];
    if (newmap.contains(elt.id())) {
      // not deleting this one
      n ++;
    } else {
      // deleting this one
      beginRemoveRows(QModelIndex(), n, n);
      d->elements.removeAt(n);
      endRemoveRows();
      N --;
    }
  }
  qDebug() << "Deleted some => " << N;

  QMap<int, int> oldmap;
  N = d->elements.size();
  for (int n=0; n<N; n++)
    oldmap[d->elements[n].id()] = n;

  for (int id: newmap.keys()) {
    if (oldmap.contains(id)) {
      // modify existing row
      int n = oldmap[id];
      d->elements[n] = newmap[id];
      emit dataChanged(index(n, 0), index(n, int(Column::N)));
      newmap.remove(id);
      qDebug() << "edited row" << n << id << d->elements[n].report();
    }
  }

  // any remaining keys must be added
  if (newmap.isEmpty())
    return;
  
  beginInsertRows(QModelIndex(), N, N+newmap.size()-1);
  for (auto const &elt: newmap) {
    qDebug() << "adding" << elt.report();
    d->elements.append(elt);
  }
  endInsertRows();
}

QList<QStringList> PartList::asTable() const {
  QMap<QString, QStringList> map;
  QStringList hdr;
  hdr << "Ref." << "Value" << "Pkg." << "Vendor" << "Cat.#"  << "Notes";
  map[""] = hdr;
  for (Element const &elt: d->elements) {
    Element::Info const &info = elt.info();
    QStringList line;
    line << elt.name() << elt.value()
         << info.package << info.vendor << info.partno << info.notes;
    map[elt.name()] = line;
  }
  return map.values();
}
