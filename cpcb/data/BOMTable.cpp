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

QStringList BOMRow::header(bool compact) {
  QStringList hdr{
    "Designator",
    "Value",
    "Footprint",
    "Mfg.",
    "Mfg. Part No.",
    "Vendor",
    "Vendor Cat. No.",
    "Notes"
  };
  if (compact)
    hdr.insert(1, "Qty.");
  return hdr;
}

QList<Group::Attribute> BOMRow::attributeOrder() {
  static QList<Group::Attribute> attrs{
    Group::Attribute::Footprint,
    Group::Attribute::Manufacturer,
    Group::Attribute::PartNo,
    Group::Attribute::Vendor,
    Group::Attribute::CatNo,
    Group::Attribute::Notes,
  };
  return attrs;
}
  
QStringList BOMRow::toStringList() const {
  QStringList lst{ref, ref.startsWith("J") ? QString() : value.trimmed()};
  for (auto attr: attributeOrder())
    lst << attributes[attr];
  return lst;
}

BOMRow BOMRow::fromStringList(QStringList const &lst) {
  BOMRow row;
  row.ref = lst[0];
  row.value = lst[1];
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
  /* For rows to be combined, everything other than the REF needs to
     be identical, including notes. */
  QMap<QStringList, QStringList> refs;
  for (QStringList row: table) {
    QString ref = row[0];
    row[0] = PartNumbering::prefix(ref);
    refs[row] << ref;
  }
  table.clear();
  for (QStringList row: refs.keys()) {
    QStringList rr = refs[row];
    row[0] = PartNumbering::compactRefs(rr);
    row.insert(1, QString::number(rr.size()));
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
              return PartNumbering::lessThan(a[0], b[0]); });
  return table;
}

bool BOMTable::saveCSV(QString fn, bool compact, QStringList *universe) const {
  QList<QStringList> table = toList(compact, universe);
  table.insert(0, BOMRow::header(compact));
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
  int hdrlen = BOMRow::header(false).size();
  for (QStringList row: list) {
    if (row.size()==hdrlen) {
      // simple row
      table << BOMRow::fromStringList(row);
    } else if (row.size()==hdrlen+1) {
      row.removeAt(1); // remove qty
      QStringList refs = PartNumbering::unpackRefs(row[0]);
      for (QString ref: refs) {
        row[0] = ref;
        table << BOMRow::fromStringList(row);
      }
    } else {
      return BOMTable(); // error
    }
  }
  return table;
}

BOMTable BOMTable::fromCSV(QString fn, QString &error) {
  QFile f(fn);
  error = "";
  if (!f.open(QFile::ReadOnly)) {
    error = "Cannot open file";
    return BOMTable();
  }
  QString csv = QTextStream(&f).readAll();
  QList<QStringList> table = CSV::decode(csv);
  if (table.isEmpty()) {
    error = "Empty CSV"; 
    return BOMTable();
  }
  qDebug() << "Read: ";
  for (QStringList row: table)
    qDebug() << "  Row: " << row;
  if (table[0] != BOMRow::header(false) && table[0] != BOMRow::header(true)) {
    error = "Mismatched header";
    qDebug() << "Expected: " << BOMRow::header(false);
    qDebug() << "Got:      " << table[0];
    return BOMTable();
  }
  table.removeAt(0); // drop header
  return fromList(table);
}

QString BOMTable::verify(Group const &root) const {
  QSet<QString> allrefs;
  for (int k: root.keys()) {
    Object const &o(root.object(k));
    if (o.isGroup())
      allrefs << o.asGroup().ref;
  }

  QList<QString> surprises;
  for (BOMRow const &row: *this) 
    if (!allrefs.contains(row.ref))
      surprises << row.ref;
  if (surprises.isEmpty())
    return QString();
  else
    return "Unexpected refs: " + surprises.join(", ");
}
  
