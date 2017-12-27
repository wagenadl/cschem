// Junction.cpp

#include "Junction.h"
#include "IDFactory.h"
#include <QPoint>

class JunctionData: public QSharedData {
public:
  JunctionData(): id(IDFactory::instance().newId()) { }
public:
  QPoint position;
  int id;
};

Junction::Junction() {
  d = new JunctionData();
}

Junction::Junction(Junction const &o) {
  d = o.d;
}

Junction &Junction::operator=(Junction const &o) {
  d = o.d;
  return *this;
}

Junction::~Junction() {
}


Junction::Junction(QXmlStreamReader &src): Junction() {
  auto a = src.attributes();
  d->position = QPoint(a.value("x").toInt(), a.value("y").toInt());
  d->id = a.value("id").toInt();
  src.skipCurrentElement();
}

QPoint Junction::position() const {
  return d->position;
}

int Junction::id() const {
  return d->id;
}

void Junction::setPosition(QPoint p) {
  d->position = p;
}

void Junction::setId(int id) {
  d->id = id;
}

QXmlStreamReader &operator>>(QXmlStreamReader &sr, Junction &c) {
  c = Junction(sr);
  return sr;
}
  
QXmlStreamWriter &operator<<(QXmlStreamWriter &sr, Junction const &c) {
  sr.writeStartElement("junction");
  sr.writeAttribute("x", QString::number(c.position().x()));
  sr.writeAttribute("y", QString::number(c.position().y()));
  sr.writeAttribute("id", QString::number(c.id()));
  sr.writeEndElement();
  return sr;
};

