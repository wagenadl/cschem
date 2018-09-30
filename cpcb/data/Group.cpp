// Group.cpp

#include "Group.h"
#include "Object.h"
#include "Const.h"
#include "ui/ORenderer.h"
#include <QBuffer>
#include <QFileInfo>

class GData: public QSharedData {
public:
  QMap<int, Object> obj;
  int lastid;
  GData() {
    lastid = 0;
    reftextid = 0;
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
}

Group &Group::operator=(Group const &o) {
  if (this == &o)
    return *this;
  d = o.d;
  ref = o.ref;
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

int Group::ensureRefText(int gid) {
  const Group *me = this;
  if (!me->contains(gid) || !me->object(gid).isGroup())
    return 0;
  Group const &g(me->object(gid).asGroup());
  if (g.refTextId()>0)
    return g.refTextId();

  d.detach();

  Text reftext;
  reftext.layer = Layer::Silk;
  reftext.fontsize = Dim::fromInch(.05);
  reftext.orient.rot = 0;
  reftext.orient.flip = false;
  reftext.text = g.ref;
  Rect bb = g.boundingRect();
  reftext.p = Point(bb.left, bb.top - Dim::fromInch(0.05));

  int tid = insert(Object(reftext));
  object(gid).asGroup().setRefTextId(tid);
  object(tid).asText().setGroupAffiliation(gid);
  return tid;
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
    if (contains(id)) {
      if (object(id).isText() && reftextmatch(object(id).asText().text))
	reftext = object(id).asText();
      else
        g.insert(object(id));
    }
  }
  for (int id: ids)
    if (d->obj.contains(id)) 
      remove(id);
  Rect bb = g.boundingRect();
  reftext.p = Point(bb.left, bb.top - Dim::fromInch(0.05));
  int tid = insert(Object(reftext));
  g.ref = reftext.text;
  int gid = insert(Object(g));
  object(gid).asGroup().setRefTextId(tid); 
  object(tid).asText().setGroupAffiliation(gid);
  return gid;
}

QSet<int> Group::dissolveSubgroup(int gid) {
  QSet<int> ids;
  if (!contains(gid))
    return ids; // error
  if (!object(gid).isGroup())
    return ids; // error
  d.detach();
  Group const &g(object(gid).asGroup());
  int tid = g.refTextId();
  for (Object const &obj: g.d->obj) {
    int id = insert(obj);
    ids << id;
  }
  if (contains(tid)) {
    Object &obj(object(tid));
    if (obj.isText())
      obj.asText().setGroupAffiliation(0);
  }
  remove(gid);
  return ids;
}

Group Group::translated(Point p) const {
  Group res = *this;
  res.translate(p);
  return res;
}

void Group::translate(Point p) {
  d.detach();
  d->hasbbox = false;
  for (Object &o: d->obj)
    o.translate(p);
}

void Group::rotateCCW(Point p) {
  d.detach();
  d->hasbbox = false;
  for (Object &o: d->obj)
    o.rotateCCW(p);
}

void Group::rotateCW(Point p) {
  d.detach();
  d->hasbbox = false;
  for (Object &o: d->obj)
    o.rotateCW(p);
}

void Group::flipLeftRight(Dim x) {
  d.detach();
  d->hasbbox = false;
  for (Object &o: d->obj)
    o.flipLeftRight(x);
}

void Group::flipUpDown(Dim y) {
  d.detach();
  d->hasbbox = false;
  for (Object &o: d->obj)
    o.flipUpDown(y);
}

int Group::insert(Object const &o) {
  d.detach();
  d->hasbbox = false;
  d->obj.insert(++d->lastid, o);
  return d->lastid;
}

void Group::remove(int key) {
  if (d->obj.contains(key)) {
    d.detach();
    d->hasbbox = false;
    d->obj.remove(key);
  }
}

bool Group::contains(int key) const {
  return d->obj.contains(key);
}

Object const &Group::object(int key) const {
  static Object nil;
  auto it = d->obj.find(key);
  if (it!=d->obj.end())
    return *it;
  else
    return nil;
}

Object &Group::object(int key) {
  d.detach();
  d->hasbbox = false;
  return d->obj[key];
}

Object const &Group::object(NodeID const &id) const {
  static Object nil;
  if (id.isEmpty() || !contains(id[0]))
    return nil;
  Object const &obj(object(id[0]));
  if (id.size()==1)
    return obj;
  if (obj.isGroup())
    return obj.asGroup().object(id.tail());
  else
    return nil;
}

Object &Group::object(NodeID const &id) {
  static Object nil;
  if (id.isEmpty()) {
    qDebug() << "Returning nonconst reference to nil.";
    return nil;
  }
  d.detach();
  d->hasbbox = false;
  Object &obj(object(id[0]));
  if (id.size()==1)
    return obj;
  if (obj.isGroup()) {
    return obj.asGroup().object(id.tail());
  } else {
    qDebug() << "Returning nonconst reference to nil.";
    return nil;
  }
}

QList<int> Group::keys() const {
  return d->obj.keys();
}

QList<NodeID> Group::allPins() const {
  QList<NodeID> ids;
  for (int id: keys()) {
    Object const &obj(object(id));
    switch (obj.type()) {
    case Object::Type::Pad:
    case Object::Type::Hole:
      ids << NodeID().plus(id);
      break;
    case Object::Type::Group:
      for (NodeID const &id1: obj.asGroup().allPins()) {
	NodeID nid;
	nid << id;
	nid.append(id1);
	ids << nid;
      }
      break;
    default:
      break;
    }
  }
  return ids;
}

Group const &Group::parentOf(NodeID path) const {
  path.removeLast();
  return subgroup(path);
}

Group &Group::parentOf(NodeID path) {
  path.removeLast();
  return subgroup(path);
}

Group const &Group::subgroup(NodeID path) const {
  static Group nil;
  if (path.isEmpty())
    return *this;
  Object const &obj = object(path.takeFirst());
  if (obj.isGroup())
    return obj.asGroup().subgroup(path);
  else
    return nil;
}
  
Group &Group::subgroup(NodeID path) {
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
    return d->bbox;
  Rect r;
  for (Object const &o: d->obj)
    r |= o.boundingRect();
  d->bbox = r;
  d->hasbbox = true;
  return r;
}

bool Group::touches(Point p, Dim mrg) const {
  if (!boundingRect().grow(mrg/2).contains(p))
    return false;
  QList<int> ids;
  for (int id: d->obj.keys()) 
    if (object(id).touches(p, mrg))
      return true;
  return false;
}

void Group::setPinRef(int id, QString ref) {
  Object &obj(object(id));
  if (obj.isHole())
    obj.asHole().ref = ref;
  else if (obj.isPad())
    obj.asPad().ref = ref;
  else
    return;
  
  Nodename refn("-", ref);
  if (refn.hasPinName()) {
    // must steal from others
    QString name = refn.pinName();
    for (int id1: keys()) {
      if (id1==id)
        continue;
      Nodename refn1("-", pinName(id1));
      if (refn1.hasPinNumber() && refn1.hasPinName() && refn1.pinName()==name) {
        // steal
        QString num1 = QString::number(refn1.pinNumber());
        Object &obj1(object(id1));
        if (obj1.isPad())
          obj1.asPad().ref = num1;
        else if (obj1.isHole())
          obj1.asHole().ref = num1;
      }
    }
  }
}

QString Group::pinName(int id) const {
  if (!d->obj.contains(id))
    return "";
  Object const &obj(object(id));
  switch (obj.type()) {
    case Object::Type::Hole:
      return obj.asHole().ref;
    case Object::Type::Pad:
      return obj.asPad().ref;
    default:
      return "";
  }
  return "";
}
  

int Group::pinID(QString s) const {
  for (int id: keys()) {
    Object const &obj = object(id);
    if (obj.isHole()) {
      if (obj.asHole().ref==s)
      return id;
    } else if (obj.isPad()) {
      if (obj.asPad().ref==s)
        return id;
    }
  }
  return -1;
}

QStringList Group::pinNames() const {
  QMap<QString, int> names;
  for (int id: keys()) {
    Object const &obj = object(id);
    switch (obj.type()) {
    case Object::Type::Hole:
      names[obj.asHole().ref] = id;
      break;
    case Object::Type::Pad:
      names[obj.asPad().ref] = id;
      break;
    default:
      break;
    }
  }
  return QStringList(names.keys());
}

Point Group::anchor() const {
  QStringList names = pinNames();
  if (names.isEmpty())
    return boundingRect().center();
  else
    return pinPosition(names.first());
}

Point Group::pinPosition(QString name) const {
  for (int id: keys()) {
    Object const &obj = object(id);
    switch (obj.type()) {
    case Object::Type::Hole: {
      Hole const &h(obj.asHole());
      if (h.ref==name)
	return h.p;
    } break;
    case Object::Type::Pad: {
      Pad const &h(obj.asPad());
      if (h.ref==name)
	return h.p;
    } break;
    default:
      break;
    }
  }
  return Point();
}  

QSet<Point> Group::allPoints() const {
  QSet<Point> pp;
  for (int id: keys()) 
    pp |=  object(id).allPoints();
  return pp;
}

QSet<Point> Group::allPoints(Layer l) const {
  QSet<Point> pp;
  for (int id: keys()) 
    pp |=  object(id).allPoints(l);
  return pp;
}

      

QSet<Point> Group::pinPoints() const {
  QSet<Point> pp;
  for (int id: keys()) {
    Object const &obj = object(id);
    switch (obj.type()) {
    case Object::Type::Hole:
      pp << obj.asHole().p;
      break;
    case Object::Type::Pad:
      pp << obj.asPad().p;
      break;
    case Object::Type::Group: 
      pp |= obj.asGroup().pinPoints();
      break;
    default:
      break;
    }
  }
  return pp;
}  

QSet<Point> Group::pinPoints(Layer l) const {
  QSet<Point> pp;
  for (int id: keys()) {
    Object const &obj = object(id);
    switch (obj.type()) {
    case Object::Type::Hole:
      if (l==Layer::Top || l==Layer::Bottom)
	pp << obj.asHole().p;
      break;
    case Object::Type::Pad: {
      Pad const &pad(obj.asPad());
      if (l==pad.layer)
	pp << pad.p;
    } break;
    case Object::Type::Group: 
      pp |= obj.asGroup().pinPoints(l);
      break;
    default:
      break;
    }
  }
  return pp;
}  

QList<int> Group::objectsAt(Point p, Dim mrg) const {
  QList<int> ids;
  for (int id: d->obj.keys()) 
    if (object(id).touches(p, mrg))
      ids << id;
  return ids;
}

QStringList Group::nodePath(NodeID const &ids) const {
  QStringList res;
  res << ref;
  if (ids.isEmpty())
    return res;
  int id = ids.first();
  Object const &obj(object(id));
  QString name;
  switch (obj.type()) {
  case Object::Type::Group:
    res += obj.asGroup().nodePath(ids.tail());
    break;
  case Object::Type::Pad:
    res << obj.asPad().ref;
    break;
  case Object::Type::Hole:
    res << obj.asHole().ref;
    break;
  default:
    break;
  }
  return res;
}

Nodename Group::nodeName(NodeID const &ids) const {
  QStringList sum;
  for (QString const &x: nodePath(ids))
    if (!x.isEmpty())
      sum << x;
  if (sum.isEmpty())
    return Nodename("", "");
  else if (sum.size()==1)
    return Nodename(sum.first(), "");
  else
    return Nodename(sum[sum.size()-2], sum.last());
}

QString Group::humanName(NodeID const &ids) const {
  if (ids.isEmpty())
    return ref;
  int id = ids.first();
  Object const &obj(object(id));
  QString name;
  switch (obj.type()) {
  case Object::Type::Group:
    name = obj.asGroup().humanName(ids.tail());
    break;
  case Object::Type::Pad:
    name = "pin " + obj.asPad().ref;
    break;
  case Object::Type::Hole:
    name = "pin " + obj.asHole().ref;
    break;
  default:
    break;
  }
  if (!ref.isEmpty() && !name.isEmpty())
    return name + " of " + ref;
  else
    return name + ref;
}

NodeID Group::nodeAt(Point p, Dim mrg, Layer lay, bool notrace) const {
  NodeID ids;
  for (int id: d->obj.keys()) {
    Object const &obj(object(id));
    if (obj.touches(p, mrg)) {
      switch (obj.type()) {
      case Object::Type::Group:
	ids = obj.asGroup().nodeAt(p, mrg, lay, true); // no traces inside groups
	ids.push_front(id);
	return ids;
      case Object::Type::Hole:
	return NodeID().plus(id);
      case Object::Type::Pad:
        if (lay==Layer::Invalid || lay==obj.asPad().layer) 
	  return NodeID().plus(id);
        break;
      case Object::Type::Trace:
        if (!notrace && ids.isEmpty()
	    && (lay==obj.asTrace().layer
		|| (lay==Layer::Invalid && obj.asTrace().layer!=Layer::Silk))) {
          ids.clear();
	  ids << id; // this could still be overwritten!
        }
        break;
      case Object::Type::Plane:
        if (!notrace && ids.isEmpty()
	    && (lay==obj.asPlane().layer
		|| lay==Layer::Invalid)) {
          ids.clear();
	  ids << id; // this could still be overwritten!
        }
        break;
      default:
        break;
      }
    }
  }
  return ids;
}
  

QXmlStreamWriter &operator<<(QXmlStreamWriter &s, Group const &t) {
  if (t.isEmpty()) {
    qDebug() << "(Empty group; not writing to xml)";
    return s;
  }
  s.writeStartElement("group");
  s.writeAttribute("ref", t.ref);
  if (!t.notes.isEmpty())
    s.writeAttribute("notes", t.notes);
  if (!t.pkg.isEmpty())
    s.writeAttribute("pkg", t.pkg);
  for (Object const &o: t.d->obj) {
    if (o.isGroup()) {
      s.writeStartElement("gr");
      int id = o.asGroup().refTextId();
      if (id>0 && t.d->obj.contains(id)) 
        s << t.object(id);
      s << o;
      s.writeEndElement();
    } else if (o.isText()
	       && (o.asText().groupAffiliation()>0 // let the group handle this
		   || o.asText().text.isEmpty()) // drop invisible empty texts
	       ) {
      // do nothing
    } else { // are there other case sthat should be dropped?
      s << o;
    }
  }
  s.writeEndElement();
  return s;
}

bool Group::saveComponent(int id, QString fn) {
  if (!d->obj.contains(id))
    return false;
  Object const &obj(object(id));
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
      sw.writeAttribute("pkg", QFileInfo(fn).baseName());
      sw.writeDefaultNamespace("http://www.danielwagenaar.net/cpcb-ns.html");
  
      Group const &grp(obj.asGroup());
      int refid = grp.refTextId();
      Text reftext;
      if (contains(refid)) {
	Object const &ref(object(refid));
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
  QString pkg;
  if (s.isStartElement())
    pkg = s.attributes().value("pkg").toString();
  while (!s.atEnd()) {
    s.readNext();
    if (s.isStartElement()) {
      if (s.name()=="group") {
        Object o;
        s >> o;
	if (o.asGroup().pkg=="")
	  o.asGroup().pkg = pkg;
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
	if (sr.isStartElement() && sr.prefix()=="cpcb" && sr.name()=="part") 
	  return readGroupAndRef(sr, *this);
      }
    }
  }
  return 0;
}

QXmlStreamReader &operator>>(QXmlStreamReader &s, Group &t) {
  t = Group();
  auto a = s.attributes();
  t.ref = a.value("ref").toString();
  t.notes = a.value("notes").toString();
  t.pkg = a.value("pkg").toString();
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
  if (a.hasAttribute("o"))
    t.translate(Point::fromString(a.value("o").toString()));

  if (t.isEmpty())
    qDebug() << "Empty group read from xml";
  // now at end of group element
  return s;
}

QDebug operator<<(QDebug d, Group const &t) {
  d << "Group(" << t.ref;
  for (Object const &o: t.d->obj)
    d << "    " << o << "\n";
  d << ")";
  return d;
}

NodeID Group::findNodeByName(Nodename name) const {
  for (int id: d->obj.keys()) {
    Object const &obj(d->obj[id]);
    if (obj.isGroup()) {
      Group const &grp(obj.asGroup());
      if (grp.ref.isEmpty()) {
	NodeID nid = grp.findNodeByName(name);
	if (!nid.isEmpty()) {
	  nid.push_front(id);
	  return nid;
	}
      } else if (grp.ref==name.component()) {
	Nodename subn("", name.pin());
	NodeID nid = grp.findNodeByName(subn);
	if (!nid.isEmpty()) {
	  nid.push_front(id);
	  return nid;
	}
      }
    } else if (obj.isPad() || obj.isHole()) {
      Nodename n1("", obj.isPad() ? obj.asPad().ref : obj.asHole().ref);
      if (name.matches(n1)) {
	NodeID nid;
	nid << id;
	return nid;
      }
    }
  }

  return NodeID();
}

QSet<QString> Group::immediateRefs() const {
  QSet<QString> refs;
  for (int id: keys()) {
    Object const &obj = object(id);
    switch (obj.type()) {
    case Object::Type::Hole:
      refs << obj.asHole().ref;
      break;
    case Object::Type::Pad:
      refs << obj.asPad().ref;
      break;
    case Object::Type::Group: 
      refs << obj.asGroup().ref;
      break;
    default:
      break;
    }
  }
  refs.remove("");
  return refs;
}  
  
static QString altRef(QString ref) {
  if (ref.isEmpty())
    return "a";
  else if (ref[ref.size()-1]>='a' && ref[ref.size()-1]<'z')
    return ref.left(ref.size()-1) + QChar(ref[ref.size()-1].unicode()+1);
  else
    return ref + "a";
}

QSet<int> Group::merge(Group const &g) {
  QSet<QString> refs = immediateRefs();
  qDebug() << "merge" << g;
  QSet<int> ids;
  QMap<int, int> idmap; // g's id to our id
  for (int id: g.keys()) {
    int ourid = insert(g.object(id));
    ids << ourid;
    idmap[id] = ourid;
  }

  // ensure no duplicate refs
  for (int id: g.keys()) {
    Object const &obj(g.object(id));
    switch (obj.type()) {
    case Object::Type::Pad: {
      Pad &pad(object(idmap[id]).asPad());
      while (refs.contains(pad.ref))
	pad.ref = ::altRef(pad.ref);
      refs << pad.ref;
    } break;
    case Object::Type::Hole: {
      Hole &hole(object(idmap[id]).asHole());
      while (refs.contains(hole.ref))
	hole.ref = ::altRef(hole.ref);
      refs << hole.ref;
    } break;
    case Object::Type::Group: {
      Group &group(object(idmap[id]).asGroup());
      while (refs.contains(group.ref))
	group.ref = ::altRef(group.ref);
      refs << group.ref;
      if (group.refTextId()>0) {
	int txtid = idmap[group.refTextId()];
	Text &text(object(txtid).asText());
	group.setRefTextId(txtid);
	text.setGroupAffiliation(idmap[id]);
	text.text = group.ref;
      }
    } break;
    default:
      break;
    }
  }
  return ids;
}

