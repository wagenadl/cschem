// Container.cpp

#include "Container.h"
#include "IDFactory.h"
#include <QPoint>

class ContainerData: public QSharedData {
public:
  ContainerData(): id(IDFactory::instance().newId()) { }
public:
  int id;
  QString name;
  QList<int> components;
  QString type;
};

Container::Container() {
  d = new ContainerData();
}

Container::Container(Container const &o) {
  d = o.d;
}

Container &Container::operator=(Container const &o) {
  d = o.d;
  return *this;
}

Container::~Container() {
}


Container::Container(QXmlStreamReader &src): Container() {
  auto a = src.attributes();
  d->id = a.value("id").toInt();
  d->name = a.value("name").toString();
  d->type = a.value("type").toString();
  for (QString const &s: a.value("components").toString().split(","))
    d->components << s.toInt();
  src.skipCurrentElement();
}

int Container::id() const {
  return d->id;
}

void Container::setId(int id) {
  d->id = id;
}

QString Container::name() const {
  return d->name;
}

QString Container::type() const {
  return d->type;
}

QList<int> const &Container::components() const {
  return d->components;
}

QList<int> &Container::components() {
  return d->components;
}

void Container::setName(QString n) {
  d->name = n;
}

void Container::setComponents(QList<int> const &l) {
  d->components = l;
}

void Container::setType(QString t) {
  d->type = t;
}

QXmlStreamReader &operator>>(QXmlStreamReader &sr, Container &c) {
  c = Container(sr);
  return sr;
}
  
QXmlStreamWriter &operator<<(QXmlStreamWriter &sr, Container const &c) {
  sr.writeStartElement("container");
  sr.writeAttribute("id", QString::number(c.id()));
  sr.writeAttribute("name", c.name());
  sr.writeAttribute("type", c.type());
  QStringList cc;
  for (int id: c.components())
    cc << QString::number(id);
  sr.writeAttribute("components", cc.join(","));
  sr.writeEndElement();
  return sr;
};

