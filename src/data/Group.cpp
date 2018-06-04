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

int Group::insert(Object const &o) {
  d->obj[++d->lastid] = new Object(o);
  return d->lastid;
}

void Group::remove(int key) {
  if (d->obj.contains(key)) {
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
  return as_nonconst(as_const(*this).object(key));
}

QList<int> Group::keys() const {
  return d->obj.keys();
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
  // not using as_const/as_nonconst here, because I think we should
  // detach the deepest group.
  // (We're actually detaching all groups, but that doesn't matter.)
  static Group nil;
  d.detach();
  if (path.isEmpty())
    return *this;
  Object &obj = object(path.takeFirst());
  if (obj.isGroup())
    return obj.asGroup().subgroup(path);
  else
    return nil;
}

QXmlStreamWriter &operator<<(QXmlStreamWriter &s, Group const &t) {
  if (t.isEmpty()) {
    qDebug() << "Unexpectedly empty group; not writing to xml";
    return s;
  }
  s.writeStartElement("group");
  for (Object const *o: t.d->obj)
    s << *o;
  s.writeEndElement();
  return s;
}
  
QXmlStreamReader &operator>>(QXmlStreamReader &s, Group &t) {
  t = Group();
  
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
    qDebug() << "Unexpectedly empty group read from xml";
  // now at end of group element
  return s;
}

QDebug operator<<(QDebug d, Group const &t) {
  d << "Group(";
  for (Object const *o: t.d->obj)
    d << *o;
  d << ")";
  return d;
}
