// BOMClip.cpp

#include "BOMClip.h"

#include <QPainterPath>
#include <QApplication>
#include <QClipboard>
#include <QBuffer>
#include <QMimeData>
#include <QJsonArray>
#include <QJsonDocument>

BOMClip &BOMClip::instance() {
  static BOMClip cb;
  return cb;
}

BOMClip::BOMClip() {
  // we have no static information; we actually store everything in the
  // system clipboard
}


bool BOMClip::isValid() const {
  QClipboard *cb = QApplication::clipboard();
  QMimeData const *md = cb ? cb->mimeData() : 0;
  return md ? md->hasFormat(dndformat) : false;
}

QMimeData *BOMClip::createMimeData(CellList const &cells) {
  QMimeData *md = new QMimeData;
  QJsonArray array;
  for (Cell const &cell: cells) {
    QJsonArray a1;
    a1.append(QJsonValue(cell.dx));
    a1.append(QJsonValue(cell.dy));
    a1.append(QJsonValue(cell.text));
    array.append(QJsonValue(a1));
  }
  md->setData(dndformat, QJsonDocument(array).toJson());
  return md;
}

void BOMClip::store(BOMClip::CellList const &cells) {
  QMimeData *md = createMimeData(cells);
  QClipboard *cb = QApplication::clipboard();
  cb->setMimeData(md);
}

BOMClip::CellList BOMClip::parseMimeData(QMimeData const *md) {
  QByteArray ba(md->data(dndformat));
  QJsonArray array = QJsonDocument::fromJson(ba).array();
  CellList res;
  for (QJsonValue const &v: array) {
    QJsonArray ar = v.toArray();
    res << Cell(ar[0].toInt(), ar[1].toInt(), ar[2].toString());
  }
  return res;
}

BOMClip::CellList BOMClip::retrieve() const {
  if (!isValid())
    return CellList();
  QClipboard *cb = QApplication::clipboard();
  QMimeData const *md = cb->mimeData();
  return parseMimeData(md);
}

  
