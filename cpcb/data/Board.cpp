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
  s.writeStartElement("outline");
  s.writeAttribute("shape", "rect");
  s.writeAttribute("w", t.width.toString());
  s.writeAttribute("h", t.height.toString());
  s.writeEndElement(); // outline
  s.writeStartElement("options");
  s.writeAttribute("grid", t.grid.toString());
  s.writeAttribute("metric", t.metric ? "1" : "0");
  s.writeAttribute("planesvis", t.planesvisible ? "1" : "0");
  for (Layer l: t.layervisible.keys())
    s.writeAttribute(QString("layer%1vis").arg(int(l)),
		     t.layervisible[l] ? "1" : "0");
  s.writeEndElement(); // options
  s.writeStartElement("links");
  if (!t.linkedschematic.isEmpty())
    s.writeAttribute("schem", t.linkedschematic);
  s.writeEndElement(); // links
  s.writeEndElement(); // board
  return s;
}

static void readOutline(QXmlStreamReader &s, Board &t) {
  auto a = s.attributes();
  t.width = Dim::fromString(a.value("w").toString());
  t.height = Dim::fromString(a.value("h").toString());
}

static void readOptions(QXmlStreamReader &s, Board &t) {
  auto a = s.attributes();
  t.grid = Dim::fromString(a.value("grid").toString());
  t.metric = a.value("metric").toInt();
  t.planesvisible = a.value("planesvis").toInt();
  for (Layer l: t.layervisible.keys())
    t.layervisible[l] = a.value(QString("layer%1vis").arg(int(l))).toInt();
}

static void readLinks(QXmlStreamReader &s, Board &t) {
  auto a = s.attributes();
  t.linkedschematic = a.value("schem").toString();
}  

QXmlStreamReader &operator>>(QXmlStreamReader &s, Board &t) {
  t = Board();
  bool ok;
  if (s.attributes().hasAttribute("w")) {
    // old style: all-in-one
    readOutline(s, t);
    readOptions(s, t);
    readLinks(s, t);
  } else {
    // new style - with subelements <outline>, <options>, and <links>
    while (!s.atEnd()) {
      s.readNext();
      if (s.isStartElement()) {
	qDebug() << "got start" << s.name();
	if (s.name()=="outline")
	  readOutline(s, t);
	else if (s.name()=="options")
	  readOptions(s, t);
	else if (s.name()=="links")
	  readLinks(s, t);
	else
	  qDebug() << "Unexpected element in <board>: " << s.name();
	s.skipCurrentElement();
      } else if (s.isEndElement()) {
	return s;
      } else if (s.isCharacters() && s.isWhitespace()) {
      } else if (s.isComment()) {
      } else {
	qDebug() << "Unexpected entity in <board>" << s.tokenType();
      }
    }
  }
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

Dim Board::fpConOverlap() {
  return Dim::fromMils(2);
}

Dim Board::traceClearance(Dim) const {
  return Dim::fromMils(15);
}

Dim Board::padClearance(Dim, Dim) const {
  return Dim::fromMils(15);
}

Dim Board::fpConWidth(Dim, Dim) const {
  return Dim::fromMils(12);
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

