// Group.cpp

#include "Group.h"
#include "Object.h"

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

QList<int> Group::keys() const {
  return d->obj.keys();
}

QXmlStreamWriter &operator<<(QXmlStreamWriter &s, Group const &t) {
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
  // now at end of group element
  return s;
}

QDebug operator<<(QDebug d, Group const &t) {
  return d;
}
