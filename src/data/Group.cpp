// Group.cpp

#include "Group.h"
#include "Object.h"
#include "Const.h"

class GData: public QSharedData {
public:
  QMap<int, Object *> obj;
  int lastid;
  GData() {
    lastid = 0;
  }
  ~GData() {
    for (Object *o: obj) 
    delete o;
  }
  mutable bool hasbbox;
  mutable Rect bbox;
};

Group::Group(): d(new GData) {
}

Group::~Group() {
}

Group::Group(Group const &o) {
  d = o.d;
}

Group &Group::operator=(Group const &o) {
  d = o.d;
  return *this;
}

bool Group::isEmpty() const {
  return d->obj.isEmpty();
}

int Group::formSubgroup(QSet<int> const &ids) {
  d.detach();
  Group g;
  for (int id: ids)
    if (d->obj.contains(id)) 
      g.insert(*d->obj[id]);
  for (int id: ids)
    if (d->obj.contains(id)) 
      remove(id);
  return insert(Object(g));
}

void Group::dissolveSubgroup(int gid) {
  if (!d->obj.contains(gid))
    return; // error
  if (!d->obj[gid]->isGroup())
    return; // error
  d.detach();
  Group const *g(&d->obj[gid]->asGroup());
  Point p = g->origin;
  for (Object const *obj: g->d->obj) {
    int id = insert(*obj);
    d->obj[id]->translate(p);
  }
  remove(gid);
}

void Group::rotateCCW(Point p) {
  d.detach();
  d->hasbbox = false;
  p -= origin;
  for (Object *o: d->obj)
    o->rotateCCW(p);
}

void Group::rotateCW(Point p) {
  d.detach();
  d->hasbbox = false;
  p -= origin;
  for (Object *o: d->obj)
    o->rotateCW(p);
}

void Group::flipLeftRight(Dim x) {
  d.detach();
  d->hasbbox = false;
  x -= origin.x;
  for (Object *o: d->obj)
    o->flipLeftRight(x);
}

void Group::flipUpDown(Dim y) {
  d.detach();
  d->hasbbox = false;
  y -= origin.y;
  for (Object *o: d->obj)
    o->flipUpDown(y);
}

int Group::insert(Object const &o) {
  d.detach();
  d->hasbbox = false;
  d->obj[++d->lastid] = new Object(o);
  return d->lastid;
}

void Group::remove(int key) {
  if (d->obj.contains(key)) {
    d.detach();
    d->hasbbox = false;
    delete d->obj[key];
    d->obj.remove(key);
  }
}

bool Group::contains(int key) const {
  return d->obj.contains(key);
}

Object const &Group::object(int key) const {
  static Object nil;
  if (d->obj.contains(key))
    return *d->obj[key];
  else
    return nil;
}

Object &Group::object(int key) {
  d.detach();
  d->hasbbox = false;
  return as_nonconst(as_const(*this).object(key));
}

QList<int> Group::keys() const {
  return d->obj.keys();
}

Point Group::originOf(QList<int> path) const {
  if (path.isEmpty())
    return origin;
  Object const &obj = object(path.takeFirst());
  if (obj.isGroup())
    return origin + obj.asGroup().originOf(path);
  else
    return origin;
}

Group const &Group::subgroup(QList<int> path) const {
  static Group nil;
  if (path.isEmpty())
    return *this;
  Object const &obj = object(path.takeFirst());
  if (obj.isGroup())
    return obj.asGroup().subgroup(path);
  else
    return nil;
}
  
Group &Group::subgroup(QList<int> path) {
  // Not using as_const/as_nonconst here, because at every level we must
  // detach *and* stop thinking we know our bbox.
  static Group nil;
  d.detach();
  d->hasbbox = false;
  if (path.isEmpty())
    return *this;
  Object &obj = object(path.takeFirst());
  if (obj.isGroup())
    return obj.asGroup().subgroup(path);
  else
    return nil;
}

Rect Group::boundingRect() const {
  if (d->hasbbox)
    return d->bbox.translated(origin);
  Rect r;
  for (Object *o: d->obj)
    r |= o->boundingRect();
  d->bbox = r;
  d->hasbbox = true;
  return r.translated(origin);
}

bool Group::touches(Point p, Dim mrg) const {
  if (!boundingRect().grow(mrg/2).contains(p))
    return false;
  QList<int> ids;
  p -= origin;
  for (int id: d->obj.keys()) 
    if (d->obj[id]->touches(p, mrg))
      return true;
  return false;
}

QList<int> Group::objectsAt(Point p, Dim mrg) const {
  p -= origin;
  QList<int> ids;
  for (int id: d->obj.keys()) 
    if (d->obj[id]->touches(p, mrg))
      ids << id;
  return ids;
}

QXmlStreamWriter &operator<<(QXmlStreamWriter &s, Group const &t) {
  if (t.isEmpty()) {
    qDebug() << "Unexpectedly empty group; not writing to xml";
    return s;
  }
  s.writeStartElement("group");
  s.writeAttribute("o", t.origin.toString());
  s.writeAttribute("ref", t.ref);
  for (Object const *o: t.d->obj)
    s << *o;
  s.writeEndElement();
  return s;
}
  
QXmlStreamReader &operator>>(QXmlStreamReader &s, Group &t) {
  t = Group();
  bool ok;
  auto a = s.attributes();
  t.ref = a.value("ref").toString();
  t.origin = Point::fromString(a.value("o").toString(), &ok);
  if (!ok) {
    s.skipCurrentElement();
    return s;
  }  
  while (!s.atEnd()) {
    s.readNext();
    if (s.isStartElement()) {
      Object o;
      s >> o;
      t.insert(o);
    } else if (s.isEndElement()) {
      break;
    } else if (s.isCharacters() && s.isWhitespace()) {
    } else if (s.isComment()) {
    } else {
      qDebug() << "Unexpected entity in group: " << s.tokenType();
    }
  }
  if (t.isEmpty())
    qDebug() << "Empty group read from xml";
  // now at end of group element
  return s;
}

QDebug operator<<(QDebug d, Group const &t) {
  d << "Group(" << t.ref << " at " << t.origin;
  for (Object const *o: t.d->obj)
    d << "    " << *o << "\n";
  d << ")";
  return d;
}
