// BOM.cpp

#include "BOM.h"
#include "Editor.h"
#include "circuit/Circuit.h"
#include "circuit/Element.h"
#include "circuit/PartNumbering.h"
#include "data/Object.h"
#include "data/Group.h"
#include <QDebug>
#include <QTextStream>
#include "data/CSV.h"

class BOMData {
public:
  BOMData(Editor *editor): editor(editor) {
    reloadData();
  }
  void reloadData();
public:
  Editor *editor;
  QList<BOMRow> elements; // list elements are rows of the model
};

void BOMData::reloadData() {
  elements.clear();
  Group const &root(editor->pcbLayout().root());
  Circuit const &circuit(editor->linkedSchematic().circuit());
  for (int k: root.keys()) {
    Object const &o(root.object(k));
    if (o.isGroup()) {
      Group const &g(o.asGroup());
      BOMRow row;
      row.id = k;
      row.ref = g.ref;
      row.pkg = g.pkg;
      row.partno = g.partno;
      row.notes = g.notes;
      int elt = circuit.elementByName(g.ref);
      if (elt>0) {
        row.value = circuit.elements[elt].value;
        if (row.notes=="")
          row.notes = circuit.elements[elt].notes;
      }
      elements << row;
    }
  }
}

int BOM::findElement(int id) const {
  for (int r=0; r<d->elements.size(); r++)
    if (d->elements[r].id == id)
      return r;
  return -1;
}

BOM::BOM(Editor *editor):
  QAbstractTableModel(editor), // editor is our parent from QObject perspective
  d(new BOMData(editor)) {
}

BOM::~BOM() {
  delete d;
}

QVariant BOM::data(QModelIndex const &index,
                        int role) const {
  int r = index.row();
  int c = index.column();
  bool iseditdisp = role==Qt::EditRole || role==Qt::DisplayRole;
  if (!iseditdisp || r<0 || c<0 || r>=d->elements.size() || c>=int(Column::N))
    return QVariant();
  BOMRow const &row = d->elements[r];
  switch (Column(c)) {
  case Column::Id:
    return row.id;
  case Column::Ref:
    if (role==Qt::DisplayRole) 
      return PartNumbering::nameToHtml(row.ref);
    else
      return row.ref;
  case Column::Value:
    return row.value;
  case Column::Package:
    return row.pkg;
  case Column::PartNo:
    return row.partno;
  case Column::Notes:
    return row.notes;
  default:
    return QVariant();
  }
}

bool BOM::setData(QModelIndex const &index, QVariant const &value,
             int role) {
  int r = index.row();
  int c = index.column();
  bool isedit = role==Qt::EditRole;
  if (!isedit || r<0 || c<0 || r>=d->elements.size() || c>=int(Column::N))
    return false;

  BOMRow &row(d->elements[r]);
  QString txt = value.toString().trimmed();
  NodeID node; node << row.id;
  switch (Column(c)) {
  case Column::Ref:
    row.ref = txt;
    break;
  case Column::Package:
    row.pkg = txt;
    d->editor->setGroupPackage(node, txt);
    break;
  case Column::PartNo:
    row.partno = txt;
    d->editor->setGroupPartno(node, txt);
    break;
  case Column::Notes: 
    row.notes = txt;
    d->editor->setGroupNotes(node, txt);
    break;
  default:
    return false;
  }
  emit dataChanged(index, index);
  return true;
}

Qt::ItemFlags BOM::flags(QModelIndex const &index) const {
  int c = index.column();
  Qt::ItemFlags f = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  switch (Column(c)) {
  case Column::Package:
  case Column::PartNo:
  case Column::Notes:
    f |= Qt::ItemIsEditable;
    break;
  default:
    break;
  }
  return f;
}

QVariant BOM::headerData(int section, Qt::Orientation orientation,
                    int role) const {
  if (role != Qt::DisplayRole)
    return QVariant();
  if (orientation==Qt::Horizontal) {
    // working on columns
    switch (Column(section)) {
    case Column::Id:
      return QString("Id");
    case Column::Ref:
      return QString("Ref.");
    case Column::Value:
      return QString("Value");
    case Column::Package:
      return QString("Pkg.");
    case Column::PartNo:
      return QString("Part no.");
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

int BOM::columnCount(QModelIndex const &) const {
  return int(Column::N);
}

int BOM::rowCount(QModelIndex const &parent) const {
  if (parent.isValid())
    return 0;
  else
    return d->elements.size();
}

void BOM::rebuild() {
  // We could do this on a per-row basis, but not right now.
  // See PartList in CSChem for example
  beginResetModel();
  d->reloadData();
  qDebug() << "BOM rebuild" << d->elements.size();
  endResetModel();
}

QList<QStringList> BOM::asTable() const {
  QList<QStringList> table;

  for (BOMRow const &elt: d->elements) 
    table << QStringList{elt.ref, elt.value, elt.pkg, elt.partno, elt.notes};
  std::sort(table.begin(), table.end(),
            [](QStringList a, QStringList b) {
              return PartNumbering::lessThan(a[0], b[0]); });
  table.insert(0, QStringList{"Ref.", "Value", "Pkg.", "Part no.", "Notes"});
  return table;
}


bool BOM::saveAsCSV(QString fn) const {
  QList<QStringList> tbl = asTable();
  QFile f(fn);
  if (f.open(QFile::WriteOnly)) {
    QTextStream(&f) << CSV::encode(tbl);
    return true;
  } else {
    return false;
  }
}

QList<BOMRow> BOM::readAndVerifyCSV(QString fn) const {
  QFile f(fn);
  if (!f.open(QFile::ReadOnly))
    return QList<BOMRow>();
  QString csv = QTextStream(&f).readAll();
  QList<QStringList> table = CSV::decode(csv);
  if (table.isEmpty())
    return QList<BOMRow>();
  if (table[0] == QStringList{"Ref.", "Value", "Pkg.", "Part no.", "Notes"})
    table.removeAt(0); // drop header

  QSet<QString> allrefs;
  for (BOMRow const &row: d->elements)
    allrefs << row.ref;
  
  QList<BOMRow> result;
  for (QStringList row: table) {
    if (row.isEmpty())
      continue;
    if (!allrefs.contains(row[0])) {
      qDebug() << "readcsv: Surprise ref" << row[0];
      return QList<BOMRow>();
    }
    BOMRow b;
    b.id = -1; // not stored
    b.ref = row[0];
    if (row.size()>=2)
      b.value = row[1];
    if (row.size()>=3)
      b.pkg = row[2];
    if (row.size()>=4)
      b.partno = row[3];
    if (row.size()>=5)
      b.notes = row[4];
    result << b;
  }
  return result;
}
