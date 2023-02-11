// BOM.h

#ifndef BOM_H

#define BOM_H

#include <QAbstractTableModel>
#include "data/Group.h"

class BOMRow {
public:
  BOMRow();
  BOMRow(int id, Group const &g);
  void augment(Circuit const &circuit);
  QStringList toStringList() const;
  static QStringList header();
  static QList<Group::Attribute> attributeOrder();
  static BOMRow fromStringList(QStringList);
  static QList<QStringList> packList(QList<QStringList>);
  static QList<QStringList> unpackList(QList<QStringList>);
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
  QList<QStringList> toList(bool compact) const;
  static BOMTable fromList() const;
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
  QList<QStringList> asTable(bool compact) const;
  bool saveAsCSV(QString fn, bool compact) const;
  static QList<BOMRow> readAndVerifyCSV(QString fn) const;
  Qt::DropActions supportedDropActions() const override;
  class Editor *editor();
signals:
  void hasLinkedSchematic(bool);
private:
  class BOMData *d;
};

#endif
