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
  void start(QPoint fromPos, int fromId, QString fromPin);
  bool isComplete() const;
  QList<Connection> connections() const;
  QList<Element> junctions() const;
private:
  class ConnBuilderData *d;
};

#endif
