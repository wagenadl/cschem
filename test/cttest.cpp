// cttest.cpp

#include "CTItem.h"
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QApplication>

int main(int argc, char **argv) {
  QApplication app(argc, argv);
  QGraphicsView view;
  QGraphicsScene scene;
  view.setScene(&scene);
  scene.addLine(QLineF(QPointF(50, 150), QPointF(250, 150)));
  scene.addLine(QLineF(QPointF(150, 50), QPointF(150, 250)));
  CTItem *cti = new CTItem;
  cti->setType(CTItem::Type::Name);
  cti->setText("R5");
  cti->setPos(QPointF(150,150));
  scene.addItem(cti);
  view.show();
  return app.exec();
}
