// Textual.cpp

#include "Textual.h"
#include "IDFactory.h"
#include <QPoint>

Textual::Textual(QPoint pos, QString txt): position(pos), text(txt) {
  id = IDFactory::instance().newId();
}

Textual Textual::translated(QPoint delta) const {
  Textual e = *this;
  e.position += delta;
  return e;
}

QXmlStreamReader &operator>>(QXmlStreamReader &sr, Textual &c) {
  c = Textual();
  auto a = sr.attributes();
  c.id = a.value("id").toInt();
  c.position = QPoint(a.value("x").toInt(), a.value("y").toInt());
  c.text = a.value("text").toString();
  sr.skipCurrentElement();
  return sr;
}

QXmlStreamWriter &operator<<(QXmlStreamWriter &sw, Textual const &c) {
  sw.writeStartElement("text");

  sw.writeAttribute("id", QString::number(c.id));
  sw.writeAttribute("x", QString::number(c.position.x()));
  sw.writeAttribute("y", QString::number(c.position.y()));
  sw.writeAttribute("text", c.text);

  sw.writeEndElement();
  return sw;
};

QString Textual::report() const {
  return QString("text#%1: [%2 at %3,%4]")
    .arg(id).arg(text)
    .arg(position.x()).arg(position.y());
}

QDebug &operator<<(QDebug &dbg, Textual const &elt) {
  dbg << elt.report();
  return dbg;
}
