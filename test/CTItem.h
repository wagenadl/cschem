// CTItem.h

#ifndef CTITEM_H

#define CTITEM_H

#include <QGraphicsTextItem>

class CTItem: public QGraphicsObject {
  /* This item keeps a text centered around pos. */
  Q_OBJECT;
public:
  enum Type {
    Plain, Name, Value
  };
public:
  CTItem(QGraphicsItem *parent=0);
  virtual ~CTItem();
  QString text() const;
  void setText(QString);
  void setType(Type);
  QGraphicsTextItem *textItem() const; // our contained item
  QRectF boundingRect() const override;
  void paint(QPainter *, const QStyleOptionGraphicsItem *,
	     QWidget *w=0) override;
private slots:
  void updatePos();
private:
  QGraphicsTextItem *txt;
  QString lastplain;
  Type type_;
};

#endif
