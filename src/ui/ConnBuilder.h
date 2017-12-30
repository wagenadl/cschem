// ConnBuilder.h

#ifndef CONNBUILDER_H

#define CONNBUILDER_H

#include <QGraphicsItemGroup>
#include "file/Connection.h"
#include "file/Element.h"

class ConnBuilder: public QGraphicsItemGroup {
public:
  ConnBuilder(class Scene *scene);
  ~ConnBuilder();
  void start(QPointF fromPos, int fromId, QString fromPin);
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
