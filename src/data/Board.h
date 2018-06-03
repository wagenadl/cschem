// Board.h

#ifndef BOARD_H

#define BOARD_H

#include "Dim.h"
#include <QXmlStreamReader>
#include <QDebug>

class Board {
public:
  Board();
public:
  Dim width;
  Dim height;
  bool metric;
  Dim grid;
};

QDebug operator<<(QDebug, Board const &);
QXmlStreamWriter &operator<<(QXmlStreamWriter &, Board const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Board &);

#endif
