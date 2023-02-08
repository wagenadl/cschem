// BOM.h

#ifndef BOM_H

#define BOM_H

#include <QAbstractTableModel>

struct BOMRow {
  int id;
  QString ref;
  QString value;
  QString pkg;
  QString partno;
  QString notes;
};

class BOM: public QAbstractTableModel {
  Q_OBJECT;
public:
  enum class Column {
    Id=0,
    Ref,
    Value,
    Package,
    PartNo,
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
  Qt::ItemFlags flags(QModelIndex const &index) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role=Qt::DisplayRole) const override;
  int columnCount(QModelIndex const &parent=QModelIndex()) const override;
  int rowCount(QModelIndex const &parent=QModelIndex()) const override;
  int findElement(int id) const; // returns row number or -1
  void rebuild(); // regrab circuit from editor, update rows as needed.
  QList<QStringList> asTable() const;
  bool saveAsCSV(QString fn) const;
  bool saveShoppingListAsCSV(QString fn) const;
  QList<BOMRow> readAndVerifyCSV(QString fn) const; // does not store data,
  // ... merely returns it
signals:
  void hasLinkedSchematic(bool);
private:
  class BOMData *d;
};

#endif
