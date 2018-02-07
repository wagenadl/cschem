// PartList.h

#ifndef PARTLIST_H

#define PARTLIST_H

#include <QAbstractTableModel>

class PartList: public QAbstractTableModel {
public:
  enum class Column {
    Id=0,
    Name,
    Value,
    Package,
    Vendor,
    CatNo,
    Notes,
    N
  };    
public:
  PartList(class Scene *scene);
  ~PartList();
  QVariant data(QModelIndex const &index,
                int role=Qt::DisplayRole) const override;
  bool setData(QModelIndex const &index, QVariant const &value,
               int role=Qt::EditRole) override;
  Qt::ItemFlags flags(QModelIndex const &index) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role=Qt::DisplayRole) const override;
  int columnCount(QModelIndex const &parent=QModelIndex()) const override;
  int rowCount(QModelIndex const &parent=QModelIndex()) const override;
  int findElement(int id) const; // returns row number or -1
  void rebuild(); // regrab circuit from scene, update rows as needed.
  QList<QStringList> asTable() const;
private:
  class PartListData *d;
};

#endif
