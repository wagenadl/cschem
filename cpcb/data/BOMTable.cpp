// BOMTable.cpp

#include "BOMTable.h"
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


BOMRow::BOMRow(): id(-1) {
}

BOMRow::BOMRow(int id, Group const &g): id(id) {
  ref = g.ref;
  attributes = g.attributes;
}

QStringList BOMRow::header() {
  static QStringList hdr{
    "Comment",
    "Designator",
    "Footprint",
    "Mfg.",
    "Mfg. Part No.",
    "Vendor",
    "Vendor Cat. No."
  };
  return hdr;
}

QList<Group::Attribute> BOMRow::attributeOrder() {
  static QList<Group::Attribute> attrs{
    Group::Attribute::Footprint,
    Group::Attribute::Manufacturer,
    Group::Attribute::PartNo,
    Group::Attribute::Vendor,
    Group::Attribute::CatNo,
  };
  return attrs;
}
  
QStringList BOMRow::toStringList() const {
  QStringList lst{value, ref};
  for (auto attr: attributeOrder())
    lst << attributes[attr];
  return lst;
}

BOMRow BOMRow::fromStringList(QStringList const &lst) {
  BOMRow row;
  row.value = lst[0];
  row.ref = lst[1];
  int k = 2;
  for (auto attr: attributeOrder()) {
    if (lst.size()>k)
      row.attributes[attr] = lst[k];
    else
      break;
    k++;
  }
  return row;
}

void BOMRow::augment(Circuit const &circuit) {
  int elt = circuit.elementByName(ref);
  if (elt<=0)
    return;
  value = circuit.elements[elt].value;
  if (value.startsWith('"') || value.startsWith("“"))
    value = "";
  else if (value=="" && !circuit.elements[elt].subtype.contains(":"))
    value = circuit.elements[elt].subtype;
}

//////////////////////////////////////////////////////////////////////
BOMTable::BOMTable() {
}

BOMTable::BOMTable(Group const &root) {
  for (int k: root.keys()) {
    Object const &o(root.object(k));
    if (o.isGroup())
      *this << BOMRow(k, o.asGroup());
  }
}

void BOMTable::augment(Circuit const &circuit) {
  for (BOMRow &row: *this)
    row.augment(circuit);
}

static QList<QStringList> packtable(QList<QStringList> table) {
  QMap<QStringList, QStringList> refs;
  for (QStringList row: table) {
    QString ref = row[1];
    row[1] = PartNumbering::prefix(ref);
    refs[row] << ref;
  }
  table.clear();
  for (QStringList row: refs.keys()) {
    row[1] = PartNumbering::compactRefs(refs[row]);
    table << row;
  }
  return table;
}


QList<QStringList> BOMTable::toList(bool compact, QStringList *universe) const {
  QList<QStringList> table;
  for (BOMRow const &row: *this)
    if (!universe || universe->contains(row.ref))
      table << row.toStringList();

  if (compact) 
    table = packtable(table);
  std::sort(table.begin(), table.end(),
            [](QStringList a, QStringList b) {
              return PartNumbering::lessThan(a[1], b[1]); });
  return table;
}

bool BOMTable::saveCSV(QString fn, bool compact, QStringList *universe) const {
  QList<QStringList> table = toList(compact, universe);
  table.insert(0, BOMRow::header());
  QFile f(fn);
  if (f.open(QFile::WriteOnly)) {
    QTextStream(&f) << CSV::encode(table);
    return true;
  } else {
    return false;
  }
}

BOMTable BOMTable::fromList(QList<QStringList> list) {
  BOMTable table;
  for (QStringList row: list) {
    if (row.size() != BOMRow::header().size())
      return BOMTable(); // error
    QStringList refs = PartNumbering::unpackRefs(row[1]);
    for (QString ref: refs) {
      row[1] = ref;
      table << BOMRow::fromStringList(row);
    }
  }
  return table;
}

BOMTable BOMTable::fromCSV(QString fn) {
  QFile f(fn);
  if (!f.open(QFile::ReadOnly))
    return BOMTable();
  QString csv = QTextStream(&f).readAll();
  QList<QStringList> table = CSV::decode(csv);
  if (table.isEmpty())
    return BOMTable();
  if (table[0] == BOMRow::header())
    table.removeAt(0); // drop header
  return fromList(table);
}

bool BOMTable::verify(Group const &root) const {
  QSet<QString> allrefs;
  for (int k: root.keys()) {
    Object const &o(root.object(k));
    if (o.isGroup())
      allrefs << o.asGroup().ref;
  }

  for (BOMRow const &row: *this) {
    if (!allrefs.contains(row.ref)) {
      qDebug() << "BOMTable::verify: Surprise ref" << row.ref;
      return false;
    }
  }

  return true;
}
  
