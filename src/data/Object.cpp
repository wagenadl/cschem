// Object.cpp

#include "Object.h"
#include "Const.h"

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
    typ = Object::Type::Null;
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
  d->typ = Type::Null;
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

bool Object::isNull() const {
  return d->typ != Type::Null;
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

Hole const &Object::asHole() const {
  Q_ASSERT(isHole());
  return *d->hole;
}

Pad const &Object::asPad() const {
  Q_ASSERT(isPad());
  return *d->pad;
}

Trace const &Object::asTrace() const {
  Q_ASSERT(isTrace());
  return *d->trace;
}

Text const &Object::asText() const {
  Q_ASSERT(isText());
  return *d->text;
}

Group const &Object::asGroup() const {
  Q_ASSERT(isGroup());
  return *d->group;
}

Hole &Object::asHole() {
  d.detach();
  return as_nonconst(as_const(*this).asHole());
}

Pad &Object::asPad() {
  d.detach();
  return as_nonconst(as_const(*this).asPad());
}

Trace &Object::asTrace() {
  d.detach();
  return as_nonconst(as_const(*this).asTrace());
}

Text &Object::asText() {
  d.detach();
  return as_nonconst(as_const(*this).asText());
}

Group &Object::asGroup() {
  d.detach();
  return as_nonconst(as_const(*this).asGroup());
}

QDebug operator<<(QDebug d, Object const &o) {
  switch (o.type()) {
  case Object::Type::Hole:
    d << o.asHole();
    break;
  case Object::Type::Pad:
    d << o.asPad();
    break;
  case Object::Type::Text:
    d << o.asText();
    break;
  case Object::Type::Trace:
    d << o.asTrace();
    break;
  case Object::Type::Group:
    d << o.asGroup();
    break;
  case Object::Type::Null:
    d << "Object(Null)";
    break;
  }
  return d;
}

QXmlStreamWriter &operator<<(QXmlStreamWriter &s, Object const &o) {
  switch (o.type()) {
  case Object::Type::Hole:
    s << o.asHole();
    break;
  case Object::Type::Pad:
    s << o.asPad();
    break;
  case Object::Type::Text:
    s << o.asText();
    break;
  case Object::Type::Trace:
    s << o.asTrace();
    break;
  case Object::Type::Group:
    s << o.asGroup();
    break;
  case Object::Type::Null:
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

bool Object::touches(Point p, Dim mrg) const {
  switch (d->typ) {
  case Type::Null:
    return false;
  case Type::Group:
    return asGroup().touches(p, mrg);
  case Type::Trace:
    return asTrace().onSegment(p, mrg);
  default:
    return boundingRect().grow(mrg/2).contains(p);
  }
}

Rect Object::boundingRect() const {
  switch (d->typ) {
  case Type::Null:
    return Rect();
  case Type::Hole:
    return asHole().boundingRect();
  case Type::Pad:
    return asPad().boundingRect();
  case Type::Text:
    return Rect(); // NYI
  case Type::Trace:
    return asTrace().boundingRect();
  case Type::Group:
    return asGroup().boundingRect();
  }
  return Rect();
}

Layer Object::layer() const {
  switch (d->typ) {
  case Type::Pad:
    return asPad().layer;
  case Type::Text:
    return asText().layer;
  case Type::Trace:
    return asTrace().layer;
  default:
    return Layer::Invalid;
  }
}
  
void Object::translate(Point const &p) {
  switch (d->typ) {
  case Type::Hole:
    d->hole->p += p;
    break;
  case Type::Pad:
    d->pad->p += p;
    break;
  case Type::Text:
    d->text->p += p;
    break;
  case Type::Trace:
    d->trace->p1 += p;
    d->trace->p2 += p;
    break;
  case Type::Group:
    d->group->origin += p;
    break;
  default:
    break;
  }
}
