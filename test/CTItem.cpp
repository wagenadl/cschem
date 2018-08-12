// CTItem.cpp

#include "CTItem.h"
#include <QTextDocument>
#include <QDebug>
#include <QRegularExpression>

CTItem::CTItem(QGraphicsItem *parent): QGraphicsObject(parent) {
  txt = new QGraphicsTextItem(this);
  txt->setTextInteractionFlags(Qt::TextEditorInteraction);
  connect(txt->document(), &QTextDocument::contentsChange,
	  this, &CTItem::updatePos, Qt::QueuedConnection);
  updatePos();
}

CTItem::~CTItem() {
}

QString CTItem::text() const {
  return txt->document()->toPlainText();
}

void CTItem::setText(QString s) {
  txt->document()->setPlainText(s);
}

void CTItem::setType(CTItem::Type typ) {
  type_ = typ;
  updatePos();
}

QGraphicsTextItem *CTItem::textItem() const {
  return txt;
}

void CTItem::updatePos() {
  static QRegularExpression wfn("^([A-Za-z]+)((\\d+)(.(\\d+))?)?$");

  qDebug() << "updatePos" << text() << txt->boundingRect();
  QRectF r(txt->boundingRect());
  txt->setPos(-r.center());
  QString ntext = text();
  if (ntext==lastplain)
    return;
  lastplain = ntext;
  switch (type_) {
  case Type::Name: {
    auto match = wfn.match(ntext);
    if (match.hasMatch()) {
      QString html = "<i>" + match.captured(1) + "</i>"
	+ "<sub>" + match.captured(2) + "</sub>";
      qDebug() << ntext << "=>" << html;
      txt->document()->setHtml(html);
    } else {
      txt->document()->setPlainText(ntext);
    }
  } break;
  case Type::Value:
    break;
  default:
    break;
  }
}

QRectF CTItem::boundingRect() const {
  return QRectF();
}

void CTItem::paint(QPainter *, const QStyleOptionGraphicsItem *,
		   QWidget *) {
}

