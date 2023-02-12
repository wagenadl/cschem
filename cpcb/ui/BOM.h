// BOM.h

#ifndef BOM_H

#define BOM_H

#include <QAbstractTableModel>
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

class BOM: public QAbstractTableModel {
  Q_OBJECT;
public:
  enum class Column {
    Id=0,
    Ref,
    Value,
    Footprint,
    Manufacturer,
    PartNo,
    Vendor,
    CatNo,
    Notes,
    N
  };
public:
  BOM(class Editor *editor);
  ~BOM();
  QVariant data(QModelIndex const &index,
                int role=Qt::DisplayRole) const override;
  bool setData(QModelIndex const &index, QVariant const &value,
               int role=Qt::EditRole) override;
  bool setAttributeData(int rowindex, Group::Attribute attr, QVariant const &value);
  Qt::ItemFlags flags(QModelIndex const &index) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role=Qt::DisplayRole) const override;
  int columnCount(QModelIndex const &parent=QModelIndex()) const override;
  int rowCount(QModelIndex const &parent=QModelIndex()) const override;
  int findElement(int id) const; // returns row number or -1
  void rebuild(); // regrab circuit from editor, update rows as needed.
  Qt::DropActions supportedDropActions() const override;
  class Editor *editor();
  BOMTable table() const;
signals:
  void hasLinkedSchematic(bool);
private:
  class BOMData *d;
};

#endif
