// HtmlDelegate.cpp

#include "HtmlDelegate.h"

#include <QLineEdit>
#include <QDebug>
#include <QPainter>
#include <QTextDocument>

HtmlDelegate::HtmlDelegate(QObject *parent): QStyledItemDelegate(parent) {
}

QWidget *HtmlDelegate::createEditor(QWidget *parent,
                                    QStyleOptionViewItem const &/* option */,
                                    QModelIndex const &/* index */) const {
  QLineEdit *editor = new QLineEdit(parent);
  return editor;
}

void HtmlDelegate::setEditorData(QWidget *editor,
                                 QModelIndex const &index) const {
  QString value = index.model()->data(index, Qt::EditRole).toString();
  QLineEdit *e = static_cast<QLineEdit *>(editor);
  qDebug() << "HtmlDelegate::setEditorData" << value;
  e->setText(value);
}

void HtmlDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   QModelIndex const &index) const {
  QLineEdit *e = static_cast<QLineEdit *>(editor);
  QString value = e->text();
  model->setData(index, value, Qt::EditRole);
}

void HtmlDelegate::updateEditorGeometry(QWidget *editor,
                                        QStyleOptionViewItem const &option,
                                        QModelIndex const &/* index */) const {
  editor->setGeometry(option.rect);
}

void HtmlDelegate::paint(QPainter *painter, QStyleOptionViewItem const &option,
                         QModelIndex const &index) const {
  auto options = option;
  initStyleOption(&options, index);

  painter->save();

  QTextDocument doc;
  doc.setHtml(index.model()->data(index, Qt::DisplayRole).toString());

  options.text = "";
  options.widget->style()->drawControl(QStyle::CE_ItemViewItem, &option, painter);

  QSizeF docsize = doc.size();
  double space = options.rect.height() - docsize.height();
  if (space > 0) 
    options.rect.adjust(0, space/2, 0, -space/2);
  QRect clip(0, 0, options.rect.width(), options.rect.height());
  painter->translate(options.rect.left(), options.rect.top());
  doc.drawContents(painter, clip);
  qDebug() << "options.rect" << options.rect << "doc" << doc.idealWidth() << doc.size();

  painter->restore();
}

QSize HtmlDelegate::sizeHint(QStyleOptionViewItem const &option,
                             const QModelIndex &index) const {
  auto options = option;
  initStyleOption(&options, index);

  QTextDocument doc;
  doc.setHtml(index.model()->data(index, Qt::DisplayRole).toString());
  doc.setTextWidth(options.rect.width());
  qDebug() << "sizehint" << doc.idealWidth() << doc.size();
  return QSize(doc.idealWidth(), doc.size().height());
}
