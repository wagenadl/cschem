// UndoStep.h

#ifndef UNDOSTEP_H

#define UNDOSTEP_H

#include <QSet>
#include <QMap>
#include "Object.h"
#include "Layout.h"
#include "Layer.h"

struct UndoStep {
public:
  Layout layout;
  QSet<int> selection;
  QMap<Layer, QSet<Point> > selpts;
  QMap<Layer, QSet<Point> > purepts;
};

#endif
