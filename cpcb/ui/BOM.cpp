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
#include <QFile>
#include <algorithm>

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

QMap<BOM::Column, Group::Attribute> col2attr{
  { BOM::Column::Footprint, Group::Attribute::Footprint },
  { BOM::Column::Manufacturer, Group::Attribute::Manufacturer },
  { BOM::Column::PartNo, Group::Attribute::PartNo },
  { BOM::Column::Vendor, Group::Attribute::Vendor },
  { BOM::Column::CatNo, Group::Attribute::CatNo },
  { BOM::Column::Notes, Group::Attribute::Notes },
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
      row.attributes = g.attributes;
      int elt = circuit.elementByName(g.ref);
      if (elt>0) {
        row.value = circuit.elements[elt].value;
        if (row.value=="")
          row.value = circuit.elements[elt].subtype;
        if (row.attributes.value(Group::Attribute::Notes)=="")
          row.attributes[Group::Attribute::Notes] = circuit.elements[elt].notes;
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
  Column col = Column(c);
  switch (col) {
  case Column::Id:
    return row.id;
  case Column::Ref:
    if (role==Qt::DisplayRole) 
      return PartNumbering::nameToHtml(row.ref);
    else
      return row.ref;
  case Column::Value:
    return row.value;
  default:
    if (col2attr.contains(col))
      return row.attributes.value(col2attr[col]);
    else
      return QVariant();
  }
}

bool BOM::setAttributeData(int r, Group::Attribute attr, QVariant const &value) {
  BOMRow &row(d->elements[r]);
  for (int c=0; c!=int(Column::N); c++) 
    if (col2attr.contains(Column(c)) && col2attr[Column(c)]==attr) 
      return setData(index(r, c), value);
  return false;
}

bool BOM::setData(QModelIndex const &index, QVariant const &value,
             int role) {
  int r = index.row();
  int c = index.column();
  bool isedit = role==Qt::EditRole;
  if (!isedit || r<0 || c<0 || r>=d->elements.size() || c>=int(Column::N))
    return false;

  BOMRow &row(d->elements[r]);
  Column col = Column(c);
  QString txt = value.toString().trimmed();
  NodeID node; node << row.id;
  switch (Column(c)) {
  case Column::Ref:
  case Column::Value:
    return false;
  default:
    if (col2attr.contains(col)) {
      row.attributes[col2attr[col]] = txt;
      d->editor->setGroupAttribute(node, col2attr[col], txt);
    } else {
      return false;
    }
  }
  emit dataChanged(index, index);
  return true;
}

Qt::ItemFlags BOM::flags(QModelIndex const &index) const {
  int c = index.column();
  Qt::ItemFlags f = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  Column col = Column(c);
  switch (col) {
  case Column::Ref:
  case Column::Value:
    break;
  default:
    if (col2attr.contains(col))
      f |= Qt::ItemIsEditable;
    break;
  }
  return f;
}

QMap<BOM::Column, QString> columnnames{
  {BOM::Column::Id, "Id"},
  {BOM::Column::Ref, "Ref."},
  {BOM::Column::Value, "Value"},
  {BOM::Column::Footprint, "Pkg."},
  {BOM::Column::Manufacturer, "Mfg."},
  {BOM::Column::PartNo, "Part#"},
  {BOM::Column::Vendor, "Vendor"},
  {BOM::Column::CatNo, "Cat#"},
  {BOM::Column::Notes, "Notes"},
};

QVariant BOM::headerData(int section, Qt::Orientation orientation,
                    int role) const {
  if (role != Qt::DisplayRole)
    return QVariant();
  if (orientation==Qt::Horizontal) {
    // working on columns
    return QVariant(columnnames.value(Column(section)));
  } else if (orientation==Qt::Vertical) {
    // working on rows
    if (section>=0 && section<d->elements.size()) {
      QString ref = d->elements[section].ref;
      QString val = d->elements[section].value;
      if (val=="")
        return ref;
      else
        return ref + ": " + val;
    }
    //return QVariant(PartNumbering::nameToHtml(d->elements[section].ref));
  }
  return QVariant(); // this should never be shown
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
  emit hasLinkedSchematic(d->editor->linkedSchematic().isValid());
}


static QList<Group::Attribute> csvattrs{
  Group::Attribute::Footprint,
  Group::Attribute::Manufacturer,
  Group::Attribute::PartNo,
  Group::Attribute::Vendor,
  Group::Attribute::CatNo,
  Group::Attribute::Notes
};
  
static QStringList headernames{"Ref.",
                              "Value",
                              "Pkg.",
                              "Mfg.",
                              "Part#",
                              "Vendor",
                              "Cat#",
                              "Notes"};

QList<QStringList> BOM::asTable() const {
  QList<QStringList> table;
  
  for (BOMRow const &elt: d->elements) {
    QStringList row{elt.ref, elt.value};
    for (auto attr: csvattrs)
      row << elt.attributes[attr];
    table << row;
  }
   
  std::sort(table.begin(), table.end(),
            [](QStringList a, QStringList b) {
              return PartNumbering::lessThan(a[0], b[0]); });
  table.insert(0, headernames);
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

QStringList mfgvendor(BOMRow const &row) {
  return QStringList{
    row.attributes.value(Group::Attribute::Manufacturer),
    row.attributes.value(Group::Attribute::PartNo),
    row.attributes.value(Group::Attribute::Vendor),
    row.attributes.value(Group::Attribute::CatNo),
  };
}

bool BOM::saveShoppingListAsCSV(QString fn) const {
  QMap<QStringList, QStringList> partno2refs;
  for (BOMRow const &elt: d->elements) 
    partno2refs[mfgvendor(elt)] << elt.ref;
  
  QFile f(fn);
  if (f.open(QFile::WriteOnly)) {
    QTextStream ts(&f);
    ts << "\"Qty\",\"Mfg.\",\"Part no\",\"Vendor\",\"Cat#\",\"Refs\"\n";
    for (QStringList partno: partno2refs.keys()) {
      QStringList refs = partno2refs[partno];
      QString crefs
        = PartNumbering::compactRefs(refs);
      int n = refs.size();
      ts << n;
      ts << ",";
      for (QString bit: partno) {
        ts << CSV::quote(bit);
        ts << ",";
      }
      ts << CSV::quote(crefs);
      ts << "\n";
    }
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
  if (table[0] == headernames)
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
    int k = 2;
    for (auto attr: csvattrs) {
      if (row.size()>k)
        b.attributes[attr] = row[k];
      else
        break;
      k++;
    }
    result << b;
  }
  return result;
}

Qt::DropActions BOM::supportedDropActions() const {
  return Qt::CopyAction;
}
