// Schem.cpp

#include "Schem.h"
#include <QDebug>

class SchemData: public QSharedData {
public:
  SchemData() { }
public:
  Circuit circuit;
  Parts parts;
};  

Schem::Schem() {
  d = new SchemData;
}

Schem::Schem(Schem const &o) {
  d = o.d;
}

Schem::Schem(QXmlStreamReader &src): Schem() {
  while (!src.atEnd()) {
    src.readNext();
    if (src.isStartElement()) {
      if (src.name()=="circuit") {
        src >> d->circuit;
      } else if (src.name()=="parts") {
        src >> d->parts;
      } else {
        qDebug() << "Unexpected element in qschem: " << src.name();
      }
    } else if (src.isEndElement()) {
      break;
    } else if (src.isCharacters() && src.isWhitespace()) {
    } else if (src.isComment()) {
    } else {
      qDebug() << "Unexpected entity in qschem: " << src.tokenType();
    }
  }
  // now at end of schem element
}

Schem &Schem::operator=(Schem const &o) {
  d = o.d;
  return *this;
}

Schem::~Schem() {
}

Circuit const &Schem::circuit() const {
  return d->circuit;
}

Circuit &Schem::circuit() {
  d.detach();
  return d->circuit;
}

Parts const &Schem::parts() const {
  return d->parts;
}

Parts &Schem::parts() {
  d.detach();
  return d->parts;
}

QXmlStreamReader &operator>>(QXmlStreamReader &sr, Schem &c) {
  c = Schem(sr);
  return sr;
}
  
QXmlStreamWriter &operator<<(QXmlStreamWriter &sr, Schem const &c) {
  sr.writeStartElement("qschem");
  sr.writeDefaultNamespace("http://www.danielwagenaar.net/qschem-ns.html");
  sr << c.circuit();
  sr << c.parts();
  sr.writeEndElement();
  return sr;
};
