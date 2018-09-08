// ConnBuilder.h

#ifndef CONNBUILDER_H

#define CONNBUILDER_H

#include <QGraphicsItemGroup>
#include "circuit/Connection.h"
#include "circuit/Element.h"

class ConnBuilder: public QGraphicsItemGroup {
public:
  ConnBuilder(class Scene *scene);
  ~ConnBuilder();
  void startFromPin(QPointF fromPos, int fromId, QString fromPin);
  void startFromConnection(QPointF fromPos, int conId, int seg);
  bool isComplete() const;
  bool isAbandoned() const;
  QList<Connection> connections() const;
  QList<Element> junctions() const;
  void keyPress(QKeyEvent *);
  void mousePress(QGraphicsSceneMouseEvent *);
  void mouseMove(QGraphicsSceneMouseEvent *);
  void mouseRelease(QGraphicsSceneMouseEvent *);
private:
  class ConnBuilderData *d;
};

#endif
