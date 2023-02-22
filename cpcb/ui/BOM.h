// BOM.h

#ifndef BOM_H

#define BOM_H

#include <QAbstractTableModel>
#include "data/BOMTable.h"

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
