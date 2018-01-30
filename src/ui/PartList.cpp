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
  case Column::Id:
    return elt.id;
  case Column::Name:
    if (role==Qt::DisplayRole) 
      return PartNumbering::nameToHtml(elt.name);
    else
      return elt.name;
  case Column::Value:
    return elt.value;
  case Column::Vendor:
    return elt.info.vendor;
  case Column::CatNo:
    return elt.info.partno;
  case Column::Package:
    return elt.info.package;
  case Column::Notes:
    return elt.info.notes;
  default:
    return QVariant();
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
  QString txt = value.toString();
  switch (Column(c)) {
  case Column::Name:
    elt.name = txt;
    break;
  case Column::Value:
    txt = PartNumbering::prettyValue(txt, elt.name);
    elt.value = txt;
    break;
  case Column::Vendor:
    elt.info.vendor = txt;
    break;
  case Column::CatNo: 
    elt.info.partno = txt;
    break;
  case Column::Package: 
    elt.info.package = txt;
    break;
  case Column::Notes: 
    elt.info.notes = txt;
    break;
  default:
    return false;
  }
  d->scene->updateFromPartList(elt);
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
    if (newmap[n].type != Element::Type::Component)
      newmap.remove(n);
  
  qDebug() << "newmap.size" << newmap.size();
  
  int N = d->elements.size();
  qDebug() << "old.size" << N;
  int n = 0;
  while (n<N) {
    Element const &elt = d->elements[n];
    if (newmap.contains(elt.id)) {
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
    oldmap[d->elements[n].id] = n;

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
    Element::Info const &info = elt.info;
    QStringList line;
    line << elt.name << elt.value
         << info.package << info.vendor << info.partno << info.notes;
    map[elt.name] = line;
  }
  return map.values();
}
