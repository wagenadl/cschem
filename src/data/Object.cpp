// Object.cpp

#include "Object.h"

class OData: public QSharedData {
public:
  Object::Type typ;
  Hole *hole;
  Pad *pad;
  Text *text;
  Trace *trace;
  Group *group;
public:
  OData() {
    hole = 0;
    pad = 0;
    text = 0;
    trace = 0;
    group = 0;
    typ = Object::Type::Invalid;
  }
  ~OData() {
    delete hole;
    delete pad;
    delete text;
    delete trace;
    delete group;
  }
};

Object::Object(Hole const &t): Object() {
  d->hole = new Hole(t);
  d->typ = Type::Hole;
}

Object::Object(Pad const &t): Object() {
  d->pad = new Pad(t);
  d->typ = Type::Pad;
}

Object::Object(Trace const &t): Object() {
  d->trace = new Trace(t);
  d->typ = Type::Trace;
}

Object::Object(Text const &t): Object() {
  d->text = new Text(t);
  d->typ = Type::Text;
}

Object::Object(Group const &t): Object() {
  d->group = new Group(t);
  d->typ = Type::Group;
}

Object::Object(): d(new OData) {
  d->typ = Type::Invalid;
}

Object::~Object() {
}

Object::Object(Object const &o) {
  d = o.d;
}

Object &Object::operator=(Object const &o) {
  d = o.d;
  return *this;
}

bool Object::isHole() const {
  return d->typ==Type::Hole;
}

bool Object::isPad() const {
  return d->typ==Type::Pad;
}

bool Object::isTrace() const {
  return d->typ==Type::Trace;
}

bool Object::isText() const {
  return d->typ==Type::Text;
}

bool Object::isGroup() const {
  return d->typ==Type::Group;
}

Hole Object::toHole() const {
  return isHole() ? *d->hole : Hole();
}

Pad Object::toPad() const {
  return isPad() ? *d->pad : Pad();
}

Trace Object::toTrace() const {
  return isTrace() ? *d->trace : Trace();
}

Text Object::toText() const {
  return isText() ? *d->text : Text();
}

Group Object::toGroup() const {
  return isGroup() ? *d->group : Group();
}

QDebug operator<<(QDebug d, Object const &o) {
  switch (o.type()) {
  case Object::Type::Hole:
    d << o.toHole();
    break;
  case Object::Type::Pad:
    d << o.toPad();
    break;
  case Object::Type::Text:
    d << o.toText();
    break;
  case Object::Type::Trace:
    d << o.toTrace();
    break;
  case Object::Type::Group:
    d << o.toGroup();
    break;
  case Object::Type::Invalid:
    d << "Object(Invalid)";
    break;
  }
  return d;
}

QXmlStreamWriter &operator<<(QXmlStreamWriter &s, Object const &o) {
  switch (o.type()) {
  case Object::Type::Hole:
    s << o.toHole();
    break;
  case Object::Type::Pad:
    s << o.toPad();
    break;
  case Object::Type::Text:
    s << o.toText();
    break;
  case Object::Type::Trace:
    s << o.toTrace();
    break;
  case Object::Type::Group:
    s << o.toGroup();
    break;
  case Object::Type::Invalid:
    s.writeStartElement("object");
    s.writeEndElement();
    break;
  }
  return s;
}

QXmlStreamReader &operator>>(QXmlStreamReader &s, Object &o) {
  QStringRef name = s.name();
  if (name=="hole") {
    Hole t;
    s >> t;
    o = Object(t);
  } else if (name=="pad") {
    Pad t;
    s >> t;
    o = Object(t);
  } else if (name=="text") {
    Text t;
    s >> t;
    o = Object(t);
  } else if (name=="trace") {
    Trace t;
    s >> t;
    o = Object(t);
  } else if (name=="group") {
    Group t;
    s >> t;
    o = Object(t);
  } else  {
    s.skipCurrentElement();
    o = Object();
  }
  return s;
}

Object::Type Object::type() const {
  return d->typ;
}
