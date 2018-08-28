// Board.cpp

#include "Board.h"

Board::Board() {
  metric = false;
  width = Dim::fromInch(3.8);
  height = Dim::fromInch(2.5);
  grid = Dim::fromInch(0.050);
  layervisible[Layer::Silk] = true;
  layervisible[Layer::Top] = true;
  layervisible[Layer::Bottom] = true;
  planesvisible = true;
}


QXmlStreamWriter &operator<<(QXmlStreamWriter &s, Board const &t) {
  s.writeStartElement("board");
  s.writeAttribute("w", t.width.toString());
  s.writeAttribute("h", t.height.toString());
  s.writeAttribute("grid", t.grid.toString());
  s.writeAttribute("metric", t.metric ? "1" : "0");
  s.writeAttribute("planesvis", t.planesvisible ? "1" : "0");
  for (Layer l: t.layervisible.keys())
    s.writeAttribute(QString("layer%1vis").arg(int(l)),
		     t.layervisible[l] ? "1" : "0");
  if (!t.linkedschematic.isEmpty())
    s.writeAttribute("schem", t.linkedschematic);
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
  if (ok)
    t.planesvisible = a.value("planesvis").toInt(&ok);
  for (Layer l: t.layervisible.keys())
    if (ok)
      t.layervisible[l] = a.value(QString("layer%1vis").arg(int(l))).toInt(&ok);
  if (ok)
    t.linkedschematic = a.value("schem").toString();
  s.skipCurrentElement();
  return s;
}

QDebug operator<<(QDebug d, Board const &t) {
  d << "Board("
    << t.width
    << t.height
    << t.grid
    << (t.metric ? "metric" : "inch")
    << t.layervisible
    << t.planesvisible
    << t.linkedschematic
    << ")";
  return d;
}

Dim Board::traceClearance(Dim) const {
  return Dim::fromMils(15);
}

Dim Board::padClearance(Dim, Dim) const {
  return Dim::fromMils(15);
}

Dim Board::maskMargin(Dim) const {
  return Dim::fromMils(10);
}  

Dim Board::maskMargin(Dim, Dim) const {
  return Dim::fromMils(10);
}  

bool Board::isEffectivelyMetric() const {
  return grid.isNull() ? metric : grid.isMetric();
}

