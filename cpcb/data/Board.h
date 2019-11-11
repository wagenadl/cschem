// Board.h

#ifndef BOARD_H

#define BOARD_H

#include "Dim.h"
#include "Layer.h"
#include <QXmlStreamReader>
#include <QDebug>

class Board {
public:
  enum class Shape {
    Rect,
    Round,
    // Complex,
  };
public:
  Board();
  bool isEffectivelyMetric() const;
  static Dim fpConOverlap();
  static Dim traceClearance(Dim lw); // clearance for a trace with given size
  static Dim padClearance(Dim w, Dim h); // clearance for a pad with given size
  // (used for round and rectangular pads)
  static Dim maskMargin(Dim od); // margin for solder mask around object
  static Dim maskMargin(Dim w, Dim h); // margin for solder mask around object
  static Dim fpConWidth(Dim w, Dim h); // width for filled-plane connection
  QString shapeName() const;
public:
  Shape shape;
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
