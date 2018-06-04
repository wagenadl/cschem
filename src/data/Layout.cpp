// Layout.cpp

#include "Layout.h"

class LData: public QSharedData {
public:
  Board board;
  Group root;
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

  while (!s.atEnd()) {
    s.readNext();
    if (s.isStartElement()) {
      if (s.name() == "board")
	s >> t.board();
      else if (s.name() == "group")
	s >> t.root();
      else
	qDebug() << "Unexpected element in layout: " << s.name();
    } else if (s.isEndElement()) {
      break;
    } else if (s.isCharacters() && s.isWhitespace()) {
    } else if (s.isComment()) {
    } else {
      qDebug() << "Unexpected entity in layout: " << s.tokenType();
    }
  }
  s.skipCurrentElement();
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

Group const &Layout::group(QList<int> const &path) const {
  return root().subgroup(path);
}

Group &Layout::group(QList<int> const &path) {
  return root().subgroup(path);
}

