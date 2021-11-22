// Object.cpp

#include "Object.h"
#include "Const.h"

class OData: public QSharedData {
public:
  Object::Type typ;
  Hole *hole;
  Pad *pad;
  Arc *arc;
  Text *text;
  Trace *trace;
  Group *group;
  FilledPlane *plane;
  NPHole *nphole;
public:
  OData() {
    hole = 0;
    pad = 0;
    arc = 0;
    text = 0;
    trace = 0;
    group = 0;
    plane = 0;
    nphole = 0;
    typ = Object::Type::Null;
  }
  OData(OData const &o): OData() {
    if (o.hole)
      hole = new Hole(*o.hole);
    if (o.nphole)
      nphole = new NPHole(*o.nphole);
    if (o.pad)
      pad = new Pad(*o.pad);
    if (o.arc)
      arc = new Arc(*o.arc);
    if (o.text)
      text = new Text(*o.text);
    if (o.trace)
      trace = new Trace(*o.trace);
    if (o.group)
      group = new Group(*o.group);
    if (o.plane)
      plane = new FilledPlane(*o.plane);
    typ = o.typ;
  }
  OData &operator=(OData const &o) {
    if (&o == this)
      return *this;

    delete hole;
    delete nphole;
    delete pad;
    delete arc;
    delete text;
    delete trace;
    delete group;
    delete plane;
    hole = 0;
    nphole = 0;
    pad = 0;
    arc = 0;
    text = 0;
    trace = 0;
    group = 0;
    plane = 0;

    if (o.hole)
      hole = new Hole(*o.hole);
    if (o.nphole)
      nphole = new NPHole(*o.nphole);
    if (o.pad)
      pad = new Pad(*o.pad);
    if (o.arc)
      arc = new Arc(*o.arc);
    if (o.text)
      text = new Text(*o.text);
    if (o.trace)
      trace = new Trace(*o.trace);
    if (o.group)
      group = new Group(*o.group);
    if (o.plane)
      plane = new FilledPlane(*o.plane);
    typ = o.typ;
    return *this;
  }
  
  ~OData() {
    delete hole;
    delete nphole;
    delete pad;
    delete arc;
    delete text;
    delete trace;
    delete group;
    delete plane;
  }
};

Object::Object(Hole const &t): Object() {
  d->hole = new Hole(t);
  d->typ = Type::Hole;
}

Object::Object(NPHole const &t): Object() {
  d->nphole = new NPHole(t);
  d->typ = Type::NPHole;
}

Object::Object(FilledPlane const &t): Object() {
  d->plane = new FilledPlane(t);
  d->typ = Type::Plane;
}

Object::Object(Pad const &t): Object() {
  d->pad = new Pad(t);
  d->typ = Type::Pad;
}

Object::Object(Arc const &t): Object() {
  d->arc = new Arc(t);
  d->typ = Type::Arc;
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

bool Object::isNPHole() const {
  return d->typ==Type::NPHole;
}

bool Object::isPad() const {
  return d->typ==Type::Pad;
}

bool Object::isArc() const {
  return d->typ==Type::Arc;
}

bool Object::isPlane() const {
  return d->typ==Type::Plane;
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

NPHole const &Object::asNPHole() const {
  Q_ASSERT(isNPHole());
  return *d->nphole;
}

Pad const &Object::asPad() const {
  Q_ASSERT(isPad());
  return *d->pad;
}

Arc const &Object::asArc() const {
  Q_ASSERT(isArc());
  return *d->arc;
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

FilledPlane const &Object::asPlane() const {
  Q_ASSERT(isPlane());
  return *d->plane;
}

Hole &Object::asHole() {
  d.detach();
  Q_ASSERT(isHole());
  return *d->hole;
}

NPHole &Object::asNPHole() {
  d.detach();
  Q_ASSERT(isNPHole());
  return *d->nphole;
}

Pad &Object::asPad() {
  d.detach();
  Q_ASSERT(isPad());
  return *d->pad;
}

Arc &Object::asArc() {
  d.detach();
  Q_ASSERT(isArc());
  return *d->arc;
}

Trace &Object::asTrace() {
  d.detach();
  Q_ASSERT(isTrace());
  return *d->trace;
}

Text &Object::asText() {
  d.detach();
  Q_ASSERT(isText());
  return *d->text;
}

Group &Object::asGroup() {
  d.detach();
  Q_ASSERT(isGroup());
  return *d->group;
}

FilledPlane &Object::asPlane() {
  d.detach();
  Q_ASSERT(isPlane());
  return *d->plane;
}

QDebug operator<<(QDebug d, Object const &o) {
  switch (o.type()) {
  case Object::Type::Hole:
    d << o.asHole();
    break;
  case Object::Type::NPHole:
    d << o.asNPHole();
    break;
  case Object::Type::Arc:
    d << o.asArc();
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
  case Object::Type::Plane:
    d << o.asPlane();
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
  case Object::Type::NPHole:
    s << o.asNPHole();
    break;
  case Object::Type::Pad:
    s << o.asPad();
    break;
  case Object::Type::Arc:
    s << o.asArc();
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
  case Object::Type::Plane:
    s << o.asPlane();
    break;
  case Object::Type::Null:
    qDebug() << "Not writing null object";
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
  } else if (name=="nphole") {
    NPHole t;
    s >> t;
    qDebug() << "read nphole" << t;
    o = Object(t);
  } else if (name=="pad") {
    Pad t;
    s >> t;
    o = Object(t);
  } else if (name=="arc") {
    Arc t;
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
  } else if (name=="plane") {
    FilledPlane t;
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
  case Type::Arc:
    return asArc().onEdge(p, mrg);
  case Type::Plane:
    return asPlane().contains(p);
  case Type::NPHole:
    return asNPHole().contains(p);
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
  case Type::NPHole:
    return asNPHole().boundingRect();
  case Type::Pad:
    return asPad().boundingRect();
  case Type::Arc:
    return asArc().boundingRect();
  case Type::Trace:
    return asTrace().boundingRect();
  case Type::Group:
    return asGroup().boundingRect();
  case Type::Text:
    return asText().boundingRect();
  case Type::Plane:
    return asPlane().boundingRect();
    return Rect();
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
  case Type::Arc:
    return asArc().layer;
  case Type::Plane:
    return asPlane().layer;
  default:
    return Layer::Invalid;
  }
}

Object Object::translated(Point const &p) const {
  Object o1 = *this;
  o1.translate(p);
  return o1;
}
  
void Object::translate(Point const &p) {
  if (p.isNull())
    return;
  d.detach();
  switch (d->typ) {
  case Type::Hole:
    d->hole->p += p;
    break;
  case Type::NPHole:
    d->nphole->p += p;
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
    d->group->translate(p);
    break;
  case Type::Arc:
    d->arc->center += p;
    break;
  case Type::Plane:
    d->plane->perimeter.translate(p);
    break;
  default:
    break;
  }
}

void Object::rotateCCW(Point const &p0, bool nottext) {
  rotateCW(p0, nottext);
  rotateCW(p0, nottext);
  rotateCW(p0, nottext);
}

void Object::rotateCW(Point const &p0, bool nottext) {
  switch (type()) {
  case Type::Null:
    break;
  case Type::Hole:
    asHole().rotateCW(p0);
    break;
  case Type::NPHole:
    asNPHole().rotateCW(p0);
    break;
  case Type::Pad:
    asPad().rotateCW(p0);
    break;
  case Type::Arc:
    asArc().rotateCW(p0);
    break;
  case Type::Text:
    asText().rotateCW(p0, nottext);
    break;
  case Type::Trace:
    asTrace().p1.rotateCW(p0);
    asTrace().p2.rotateCW(p0);
    break;
  case Type::Plane:
    break;
  case Type::Group:
    asGroup().rotateCW(p0);
    break;
  }
}

void Object::freeRotate(int degCW, Point const &p0) {
  switch (type()) {
  case Type::Null:
    break;
  case Type::Hole:
    asHole().freeRotate(degCW, p0);
    break;
  case Type::NPHole:
    asNPHole().freeRotate(degCW, p0);
    break;
  case Type::Pad:
    asPad().freeRotate(degCW, p0);
    break;
  case Type::Arc:
    asArc().freeRotate(degCW, p0);
    break;
  case Type::Text:
    asText().freeRotate(degCW, p0);
    break;
  case Type::Trace:
    asTrace().p1.freeRotate(degCW, p0);
    asTrace().p2.freeRotate(degCW, p0);
    break;
  case Type::Plane:
    break;
  case Type::Group:
    asGroup().freeRotate(degCW, p0);
    break;
  }
}

void Object::flipLeftRight(Dim x0, bool nottext) {
  switch (type()) {
  case Type::Null:
    break;
  case Type::Hole:
    asHole().flipLeftRight(x0);
    break;
  case Type::NPHole:
    asNPHole().flipLeftRight(x0);
    break;
  case Type::Pad:
    asPad().flipLeftRight(x0);
    break;
  case Type::Arc:
    asArc().flipLeftRight(x0);
    break;
  case Type::Text:
    asText().flipLeftRight(x0, nottext);
    break;
  case Type::Trace:
    asTrace().p1.flipLeftRight(x0);
    asTrace().p2.flipLeftRight(x0);
    break;
  case Type::Plane:
    break;
  case Type::Group:
    asGroup().flipLeftRight(x0);
    break;
  }
}

void Object::flipUpDown(Dim y0, bool nottext) {
  switch (type()) {
  case Type::Null:
    break;
  case Type::Hole:
    asHole().flipUpDown(y0);
    break;
  case Type::NPHole:
    asNPHole().flipUpDown(y0);
    break;
  case Type::Pad:
    asPad().flipUpDown(y0);
    break;
  case Type::Arc:
    asArc().flipUpDown(y0);
    break;
  case Type::Text:
    asText().flipUpDown(y0, nottext);
    break;
  case Type::Trace:
    asTrace().p1.flipUpDown(y0);
    asTrace().p2.flipUpDown(y0);
    break;
  case Type::Plane:
    break;
  case Type::Group:
    asGroup().flipUpDown(y0);
    break;
  }
}

QSet<Point> Object::allPoints() const {
  QSet<Point> pp;
  switch (type()) {
  case Object::Type::Null:
    break;
  case Object::Type::NPHole:
    pp << asNPHole().p;
    break;
  case Object::Type::Hole:
    pp << asHole().p;
    break;
  case Object::Type::Pad:
    pp << asPad().p;
    break;
  case Object::Type::Arc:
    pp << asArc().center;
    break;
  case Object::Type::Text:
    break;
  case Object::Type::Trace:
    pp << asTrace().p1 << asTrace().p2;
    break;
  case Object::Type::Group:
    pp |= asGroup().allPoints();
    break;
  case Object::Type::Plane:
    break;
  }
  return pp;
}

QSet<Point> Object::allPoints(Layer lay) const {
  QSet<Point> pp;
  switch (type()) {
  case Object::Type::Null:
    break;
  case Object::Type::NPHole:
    pp << asNPHole().p;
    break;
  case Object::Type::Hole:
    if (lay==Layer::Top || lay==Layer::Bottom)
      pp << asHole().p;
    break;
  case Object::Type::Pad:
    if (lay==asPad().layer)
      pp << asPad().p;
    break;
  case Object::Type::Arc:
    if (lay==asArc().layer)
      pp << asArc().center;
    break;
  case Object::Type::Text:
    break;
  case Object::Type::Trace:
    if (lay==asTrace().layer)
      pp << asTrace().p1 << asTrace().p2;
    break;
  case Object::Type::Group:
    pp |= asGroup().allPoints(lay);
    break;
  case Object::Type::Plane:
    break;
  }
  return pp;
}

QSet<Point> Object::pinPoints() const {
  QSet<Point> pp;
  switch (type()) {
  case Object::Type::Hole:
    pp << asHole().p;
    break;
  case Object::Type::Pad:
    pp << asPad().p;
    break;
  case Object::Type::Group:
    pp |= asGroup().pinPoints();
    break;
  default:
    break;
  }
  return pp;
}

QSet<Point> Object::pinPoints(Layer lay) const {
  QSet<Point> pp;
  switch (type()) {
  case Object::Type::Hole:
    if (lay==Layer::Top || lay==Layer::Bottom)
      pp << asHole().p;
    break;
  case Object::Type::Pad:
    if (lay==asPad().layer)
      pp << asPad().p;
    break;
  case Object::Type::Group:
    pp |= asGroup().pinPoints(lay);
    break;
  default:
    break;
  }
  return pp;
}
