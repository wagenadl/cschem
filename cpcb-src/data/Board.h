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
  Dim traceClearance(Dim lw) const; // clearance for a trace with given size
  Dim padClearance(Dim w, Dim h) const; // clearance for a pad with given size
  // (used for round and rectangular pads)
  Dim maskMargin(Dim od) const; // margin for solder mask around object
  Dim maskMargin(Dim w, Dim h) const; // margin for solder mask around object
  Dim fpConWidth(Dim w, Dim h) const; // width for filled-plane connection
public:
  Dim width;
  Dim height;
  bool metric;
  Dim grid;
  QMap<Layer, bool> layervisible;
  bool planesvisible;
  QString linkedschematic;
};

QDebug operator<<(QDebug, Board const &);
QXmlStreamWriter &operator<<(QXmlStreamWriter &, Board const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Board &);

#endif
