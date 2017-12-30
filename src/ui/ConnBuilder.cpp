// ConnBuilder.cpp

#include "ConnBuilder.h"

class ConnBuilderData {
public:
  ConnBuilderData(Scene *scene): scene(scene), circ(scene->circuit) {
    isComplete = false;
    fromId = -1;
    toId = -1;
  }
public:
  Scene *scene;
  Circuit circ;
  QSet<int> junctions;
  QSet<int> connections;
  QPolygon points; // includes from and to
  int fromId; // could refer to a new junction!
  int toId; // could refer to a new junction!
  QString fromPin;
  QString toPin;
};
