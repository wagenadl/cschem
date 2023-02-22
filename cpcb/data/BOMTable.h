// BOMTable.h

#ifndef BOMTABLE_H

#define BOMTABLE_H
#include "data/Group.h"

class BOMRow {
public:
  BOMRow();
  BOMRow(int id, Group const &g);
  void augment(class Circuit const &circuit);
  QStringList toStringList() const;
  static QStringList header();
  static QList<Group::Attribute> attributeOrder();
  static BOMRow fromStringList(QStringList const &);
public:
  int id;
  QString ref;
  QString value;
  QMap<Group::Attribute, QString> attributes;
};

class BOMTable: public QList<BOMRow> {
public:
  BOMTable();
  BOMTable(Group const &root);
  void augment(Circuit const &circuit);
  QList<QStringList> toList(bool compact, QStringList *universe=0) const;
  // if universe is given, only refs in universe are exported
  bool saveCSV(QString fn, bool compact, QStringList *universe=0) const;
  static BOMTable fromList(QList<QStringList>);
  static BOMTable fromCSV(QString fn);
  bool verify(Group const &root) const; // true iff all refs in table exist in root
};

#endif
