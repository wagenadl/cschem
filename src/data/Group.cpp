// Group.cpp

#include "Group.h"
#include "Object.h"
#include "Const.h"
#include "ui/ORenderer.h"
#include <QBuffer>

class GData: public QSharedData {
public:
  QMap<int, Object *> obj;
  int lastid;
  GData() {
    lastid = 0;
    reftextid = 0;
  }
  ~GData() {
    for (Object *o: obj) 
      delete o;
  }
  int reftextid;
  mutable bool hasbbox;
  mutable Rect bbox;
};

Group::Group(): d(new GData) {
}

Group::~Group() {
}

Group::Group(Group const &o) {
  d = o.d;
  ref = o.ref;
  origin = o.origin;
}

Group &Group::operator=(Group const &o) {
  d = o.d;
  ref = o.ref;
  origin = o.origin;
  return *this;
}

bool Group::isEmpty() const {
  return d->obj.isEmpty();
}

int Group::refTextId() const {
  return d->reftextid;
}

void Group::setRefTextId(int id) {
  d.detach();
  d->reftextid = id;
}

int Group::formSubgroup(QSet<int> const &ids) {
  d.detach();
  Group g;
  Text reftext;
  reftext.layer = Layer::Silk;
  reftext.fontsize = Dim::fromInch(.05);
  reftext.orient.rot = 0;
  reftext.orient.flip = false;
  reftext.text = "X?";
  auto reftextmatch = [this](QString txt) {
    return txt == ref || QRegExp("[A-Z]([0-9]+|\\?)").exactMatch(txt);
  };
  for (int id: ids) {
    if (d->obj.contains(id)) {
      if (d->obj[id]->isText() && reftextmatch(d->obj[id]->asText().text))
        reftext = d->obj[id]->asText();
      else
        g.insert(*d->obj[id]);
    }
  }
  for (int id: ids)
    if (d->obj.contains(id)) 
      remove(id);
  Rect bb = g.boundingRect();
  reftext.p = Point(bb.left, bb.top - Dim::fromInch(0.05));
  int tid = insert(Object(reftext));
  g.ref = reftext.text;
  qDebug() << "ref" << g.ref;
  g.setRefTextId(tid); // this works, but for reasons I do not understand,
  // it does not work if I insert it first and /then/ change the reftextid.
  // Perhaps I do not actually understand all the way how detach() works?
  int gid = insert(Object(g));
  d->obj[tid]->asText().setGroupAffiliation(gid);
  qDebug() << "..." << d->obj[gid]->asGroup().ref;
  return gid;
}

void Group::dissolveSubgroup(int gid) {
  if (!d->obj.contains(gid))
    return; // error
  if (!d->obj[gid]->isGroup())
    return; // error
  d.detach();
  Group const *g(&d->obj[gid]->asGroup());
  Point p = g->origin;
  int tid = g->refTextId();
  for (Object const *obj: g->d->obj) {
    int id = insert(*obj);
    d->obj[id]->translate(p);
  }
  if (d->obj.contains(tid)) {
    Object &obj(*d->obj[tid]);
    if (obj.isText())
      obj.asText().setGroupAffiliation(0);
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

QSet<Point> Group::points() const {
  QSet<Point> pp;
  for (int id: keys()) {
    Object const &obj = object(id);
    switch (obj.type()) {
    case Object::Type::Hole:
      pp << obj.asHole().p + origin;
      break;
    case Object::Type::Pad:
      pp << obj.asPad().p + origin;
      break;
    default:
      break;
    }
  }
  return pp;
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
  for (Object const *o: t.d->obj) {
    if (o->isGroup()) {
      s.writeStartElement("gr");
      int id = o->asGroup().refTextId();
      if (id>0 && t.d->obj.contains(id)) 
        s << *t.d->obj[id];
      s << *o;
      s.writeEndElement();
    } else if (o->isText() && o->asText().groupAffiliation()>0) {
      // let the group handle this item
    } else {
      s << *o;
    }
  }
  s.writeEndElement();
  return s;
}

bool Group::saveComponent(int id, QString fn) {
  if (!d->obj.contains(id))
    return false;
  Object const &obj(*d->obj[id]);
  if (!obj.isGroup())
    return false;

  QFile file(fn);
  if (!file.open(QFile::WriteOnly)) {
    qDebug() << "Failed to open" << fn;
    return false;
  }
  QXmlStreamWriter sw(&file);
  sw.setAutoFormatting(true);
  sw.setAutoFormattingIndent(2);
  
  QByteArray svg = ORenderer::objectToSvg(obj, Dim::fromMils(50),
					  Dim::fromMils(500));
  QBuffer svgbuf(&svg);
  svgbuf.open(QBuffer::ReadOnly);
  QXmlStreamReader sr(&svgbuf);

  while (!sr.atEnd()) {
    sr.readNext();
    if (sr.isStartElement() && sr.name()=="svg") {
      sw.writeStartElement("svg");
      sw.writeAttributes(sr.attributes());
      for (auto ns: sr.namespaceDeclarations())
	sw.writeNamespace(ns.namespaceUri().toString(), ns.prefix().toString());

      sw.writeNamespace("http://www.danielwagenaar.net/cpcb-ns.html", "cpcb");
      sw.writeStartElement("cpcb:part");
      sw.writeDefaultNamespace("http://www.danielwagenaar.net/cpcb-ns.html");
  
      Group const &grp(obj.asGroup());
      int refid = grp.refTextId();
      Text reftext;
      if (d->obj.contains(refid)) {
	Object const &ref(*d->obj[refid]);
	if (ref.isText())
	  reftext = ref.asText();
      }

      sw << grp;
      sw << reftext;

      sw.writeEndElement(); // cpart
    } else {
      sw.writeCurrentToken(sr);
    }
  }
  return true;
}

static int readGroupAndRef(QXmlStreamReader &s, Group &t) {
  int gid = 0;
  int tid = 0;
  while (!s.atEnd()) {
    s.readNext();
    if (s.isStartElement()) {
      qDebug() << "rgar" << s.name();
      if (s.name()=="group") {
        Object o;
        s >> o;
        gid = t.insert(o);
      } else if (s.name()=="text") {
        Object o;
        s >> o;
        tid = t.insert(o);
      } else {
        qDebug() << "Unexpected element in gr: " << s.name();
      }
    } else if (s.isEndElement()) {
      break;
    } else if (s.isCharacters() && s.isWhitespace()) {
    } else if (s.isComment()) {
    } else {
      qDebug() << "Unexpected entity in gr: " << s.tokenType();
    }
  }
  if (gid>0 && tid>0) {
    t.object(tid).asText().setGroupAffiliation(gid);
    t.object(gid).asGroup().setRefTextId(tid);
  } else {
    if (gid<=0)
      qDebug() << "Missing group in gr";
    if (tid<=0)
      qDebug() << "Missing ref text in gr";
  }
  return gid;
}

int Group::insertComponent(QString fn) {
  QFile file(fn);
  if (file.open(QFile::ReadOnly)) {
    QXmlStreamReader sr(&file);
    while (!sr.atEnd()) {
      sr.readNext();
      if (sr.isStartElement()) {
	qDebug() << "elt" << sr.name() << sr.prefix();
	if (sr.isStartElement() && sr.prefix()=="cpcb" && sr.name()=="part") 
	  return readGroupAndRef(sr, *this);
      }
    }
  }
  return 0;
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
      if (s.name()=="gr") {
        readGroupAndRef(s, t);
      } else {
        Object o;
        s >> o;
        t.insert(o);
      }
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
