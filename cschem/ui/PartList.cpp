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
    elements = scene->circuit().elements.values();
  }
public:
  Scene *scene;
  QList<Element> elements; // list elements are rows of the model
};

int PartList::findElement(int id) const {
  for (int r=0; r<d->elements.size(); r++)
    if (d->elements[r].id == id)
      return r;
  return -1;
}

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
      return PartNumbering::nameToHtml(elt.name,
                                 elt.type==Element::Type::Component);
    else
      return elt.name;
  case Column::Value:
    return elt.value;
  case Column::Notes:
    return elt.notes;
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
  QString txt = value.toString().trimmed();
  switch (Column(c)) {
  case Column::Name:
    elt.name = txt;
    break;
  case Column::Value:
    txt = PartNumbering::prettyValue(txt, elt.name);
    elt.value = txt;
    break;
  case Column::Notes: 
    elt.notes = txt;
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

  //// Let's first see what we simply need to drop
  // Find collection of putatively meritorious elements in updated circuit
  QMap<int, Element> newmap = d->scene->circuit().elements; // id to element
  QMap<QString, int> refmap; // ref/name to id
  for (int n: newmap.keys()) {
    if (newmap[n].type == Element::Type::Component) {
      QString ref = newmap[n].name;
      if (refmap.contains(ref)) {
	// arbitrate: overwrite if this element is a container
	if (newmap[n].isContainer())
	  refmap[ref] = n;
      } else {
	refmap[ref] = n;
      }
    } else {
      newmap.remove(n);
    }
  }

  // Drop elements that have refs like "A3.7" iff there is also a corresponding
  // ref like "A3". Also drop duplicates
  for (int n: newmap.keys()) {
    QString ref = newmap[n].name;
    if (refmap[ref] == n) {
      int idx = ref.indexOf(".");
      if (idx>0 && ref.mid(idx+1).toInt()>0 && refmap.contains(ref.left(idx)))
	newmap.remove(n);
    } else {
      newmap.remove(n);
    }
  }

  //// Compare our old list to the new list and remove debris
  int N = d->elements.size();
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

  //// Update elements that already existed
  QMap<int, int> oldmap;
  N = d->elements.size();
  for (int n=0; n<N; n++)
    oldmap[d->elements[n].id] = n;

  // Drop entries from newmap to mark that they have been processed
  for (int id: newmap.keys()) {
    if (oldmap.contains(id)) {
      // modify existing row
      int n = oldmap[id];
      if (d->elements[n] != newmap[id]) {
        d->elements[n] = newmap[id];
        emit dataChanged(index(n, 0), index(n, int(Column::N)));
      }
      newmap.remove(id);
    }
  }

  //// Insert new elements
  // any keys remaining in newmap must be added
  if (newmap.isEmpty())
    return;
  
  beginInsertRows(QModelIndex(), N, N+newmap.size()-1);
  for (auto const &elt: newmap) {
    d->elements.append(elt);
  }
  endInsertRows();
}

QList<QStringList> PartList::asTable() const {
  QList<QStringList> table;
  
  for (Element const &elt: d->elements) 
    table << QStringList{elt.name, elt.value, elt.notes};
  std::sort(table.begin(), table.end(),
            [](QStringList a, QStringList b) {
              return PartNumbering::lessThan(a[0], b[0]); });
  table.insert(0, QStringList{"Ref.", "Value", "Notes"});
  return table;
}

