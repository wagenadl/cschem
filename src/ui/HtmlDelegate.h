// HtmlDelegate.h

#ifndef HTMLDELEGATE_H
#define HTMLDELEGATE_H

#include <QStyledItemDelegate>

class HtmlDelegate : public QStyledItemDelegate {
public:
  HtmlDelegate(QObject *parent = 0);
  QWidget *createEditor(QWidget *parent, QStyleOptionViewItem const &option,
                        QModelIndex const &index) const override;
  void setEditorData(QWidget *editor, QModelIndex const &index) const override;
  void setModelData(QWidget *editor, QAbstractItemModel *model,
                    QModelIndex const &index) const override;
  void updateEditorGeometry(QWidget *editor,
                            QStyleOptionViewItem const &option,
                            QModelIndex const &index) const override;
  void paint(QPainter *painter, QStyleOptionViewItem const &option,
             QModelIndex const &index) const;
  QSize sizeHint(QStyleOptionViewItem const &option, QModelIndex const &index) const;
};

#endif
