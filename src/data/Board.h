// Board.h

#ifndef BOARD_H

#define BOARD_H

#include "Dim.h"
#include "Layer.h"
#include <QXmlStreamReader>
#include <QDebug>

class Board {
public:
  Board();
  bool isEffectivelyMetric() const;
public:
  Dim width;
  Dim height;
  bool metric;
  Dim grid;
  QMap<Layer, bool> layervisible;
  bool planesvisible;
};

QDebug operator<<(QDebug, Board const &);
QXmlStreamWriter &operator<<(QXmlStreamWriter &, Board const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Board &);

#endif
