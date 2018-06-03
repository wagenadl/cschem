// Board.cpp

#include "Board.h"

Board::Board() {
  metric = false;
  width = Dim::fromInch(3.8);
  height = Dim::fromInch(2.5);
  grid = Dim::fromInch(0.050);
}


QXmlStreamWriter &operator<<(QXmlStreamWriter &s, Board const &t) {
  s.writeStartElement("board");
  s.writeAttribute("w", t.width.toString());
  s.writeAttribute("h", t.height.toString());
  s.writeAttribute("grid", t.grid.toString());
  s.writeAttribute("metric", t.metric ? "1" : "0");
  s.writeEndElement();
  return s;
}
  
QXmlStreamReader &operator>>(QXmlStreamReader &s, Board &t) {
  t = Board();
  bool ok;
  auto a = s.attributes();
  t.width = Dim::fromString(a.value("w").toString(), &ok);
  if (ok)
    t.height = Dim::fromString(a.value("h").toString(), &ok);
  if (ok)
    t.grid = Dim::fromString(a.value("grid").toString(), &ok);
  if (ok)
    t.metric = a.value("metric").toInt(&ok);
  s.skipCurrentElement();
  return s;
}

QDebug operator<<(QDebug d, Board const &t) {
  d << "Board("
    << t.width
    << t.height
    << t.grid
    << (t.metric ? "metric" : "inch")
    << ")";
  return d;
}

