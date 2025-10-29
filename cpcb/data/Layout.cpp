// Layout.cpp

#include "Layout.h"

class LData: public QSharedData {
public:
  LData() { valid = true; }
public:
  Board board;
  Group root;
  bool valid;
};

Layout::Layout(): d(new LData) {
}

Layout::Layout(Layout const &o) {
  d = o.d;
}

Layout &Layout::operator=(Layout const &o) {
  d = o.d;
  return *this;
}

Layout::~Layout() {
}

QXmlStreamWriter &operator<<(QXmlStreamWriter &s, Layout const &t) {
  s.writeStartElement("cpcb");
  s.writeDefaultNamespace("http://www.danielwagenaar.net/cpcb-ns.html");
  s << t.board();
  s << t.root();
  s.writeEndElement();
  return s;
}
  
QXmlStreamReader &operator>>(QXmlStreamReader &s, Layout &t) {
  t = Layout();
  bool gotboard = false;
  //  bool gotroot = false;
  bool gottrouble = false;
  while (!s.atEnd()) {
    s.readNext();
    if (s.isStartElement()) {
      if (s.name() == QStringLiteral("board")) {
	s >> t.board();
        gotboard = true;
      } else if (s.name() == QStringLiteral("group")) {
	s >> t.root();
        // gotroot = true;
      } else {
	qDebug() << "Unexpected element in layout: " << s.name();
        gottrouble = true;
      }
    } else if (s.isEndElement()) {
      break;
    } else if (s.isCharacters() && s.isWhitespace()) {
    } else if (s.isComment()) {
    } else {
      qDebug() << "Unexpected entity in layout: " << s.tokenType();
      gottrouble = true;
    }
  }
  s.skipCurrentElement();
  if (gottrouble || !gotboard)
    t.invalidate();
  return s;
}

QDebug operator<<(QDebug d, Layout const &t) {
  d << "Layout("
    << t.board() << ";\n"
    << t.root()
    << ")";
  return d;
}

Board const &Layout::board() const {
  return d->board;
}

Group const &Layout::root() const {
  return d->root;
}

Board &Layout::board() {
  d.detach();
  return d->board;
}

Group &Layout::root() {
  d.detach();
  return d->root;
}

bool Layout::isValid() const {
  return d->valid;
}

void Layout::invalidate() {
  d->valid = false;
}

  
