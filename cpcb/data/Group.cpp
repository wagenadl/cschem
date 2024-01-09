// Group.cpp

#include "Group.h"
#include "Object.h"
#include "Const.h"
#include "ui/ORenderer.h"
#include <QBuffer>
#include <QFileInfo>
#include <QRegularExpression>

static QMap<Group::Attribute, QString> xmlnames{
  { Group::Attribute::Footprint, "pkg" },
  { Group::Attribute::Manufacturer, "mfg" },
  { Group::Attribute::PartNo, "partno" },
  { Group::Attribute::Vendor, "vendor" },
  { Group::Attribute::CatNo, "catno" },
  { Group::Attribute::Notes, "notes" },
};

class GData: public QSharedData {
public:
  QMap<int, Object> obj;
  int lastid;
  GData() {
    lastid = 0;
    reftextid = 0;
    nominalrotation = 0;
  }
  int reftextid;
  int nominalrotation;
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
  attributes = o.attributes;
}

Group &Group::operator=(Group const &o) {
  if (this == &o)
    return *this;
  d = o.d;
  ref = o.ref;
  attributes = o.attributes;
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
  qDebug() << "ensurereftext" << gid;
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

void Group::freeRotate(int degcw, Point const &p) {
  d.detach();
  d->hasbbox = false;
  d->nominalrotation -= degcw;
  for (Object &o: d->obj)
    o.freeRotate(degcw, p);
}  

void Group::rotateCCW(Point p) {
  d.detach();
  d->hasbbox = false;
  d->nominalrotation += 90;
  for (Object &o: d->obj)
    o.rotateCCW(p);
}

void Group::rotateCW(Point p) {
  d.detach();
  d->hasbbox = false;
  d->nominalrotation -= 90;  
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

Group const &Group::subByRef(QString ref) const {
  static Group nil;
  for (auto it=d->obj.begin(); it!=d->obj.end(); ++it) {
    Object const &obj(it.value());
    if (obj.isGroup() && obj.asGroup().ref==ref) {
      return obj.asGroup();
    }
  }
  return nil;
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

QSet<Point> Group::altCoords() const {
  QSet<Point> pp;
  for (int id: keys()) 
    pp |= object(id).altCoords();
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
  return nodeName(ids).humanName();
}

NodeID Group::nodeAt(Point p, Dim mrg, Layer lay, bool notrace,
                     Dim *distance_return) const {
  NodeID ids;
  bool gotplane = false;
  Dim dist = Dim::infinity();
  for (int id: d->obj.keys()) {
    Object const &obj(object(id));
    if (obj.touches(p, mrg)) {
      switch (obj.type()) {
      case Object::Type::Group: {
        Group const &g = obj.asGroup();
        Dim dist1;
	NodeID ids1 = g.nodeAt(p, mrg, lay, notrace, &dist1);
        if (dist1<dist) {
          gotplane = false;
          ids = ids1;
          ids.push_front(id);
          dist = dist1;
        }
      } break;
      case Object::Type::Hole: {
        Dim dist1 = p.distance(obj.asHole().p);
        if (dist1 < dist) {
          gotplane = false;
          ids = NodeID().plus(id);
          dist = dist1;
        }
      } break;
      case Object::Type::Pad:
        if (lay==Layer::Invalid || lay==obj.asPad().layer) {
          Dim dist1 = p.distance(obj.asPad().p);
          if (dist1 < dist) {
            gotplane = false;
            ids = NodeID().plus(id);
            dist = dist1;
          }
        }
        break;
      case Object::Type::Trace:
        if (!notrace
            && (ids.isEmpty() || gotplane)
	    && (lay==obj.asTrace().layer
		|| (lay==Layer::Invalid
		    && layerIsCopper(obj.asTrace().layer)))) {
          gotplane = false;
          ids = NodeID().plus(id); // this could still be overwritten!
        }
        break;
      case Object::Type::Plane:
        if (!notrace && ids.isEmpty()
	    && (lay==obj.asPlane().layer || lay==Layer::Invalid)) {
          gotplane = true;
          ids = NodeID().plus(id); // this could still be overwritten!
        }
        break;
      default:
        break;
      }
    }
  }
      if (distance_return)
        *distance_return = dist;
  return ids;
}

QXmlStreamWriter &operator<<(QXmlStreamWriter &s, Group const &t) {
  if (t.isEmpty()) {
    qDebug() << "(Empty group; not writing to xml)";
    return s;
  }
  s.writeStartElement("group");
  if (!t.ref.isEmpty())
    s.writeAttribute("ref", t.ref);
  for (Group::Attribute attr: t.attributes.keys())
        s.writeAttribute(xmlnames[attr], t.attributes[attr]);
  if (t.d->nominalrotation)
    s.writeAttribute("nomrot", QString::number(t.d->nominalrotation));
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

bool Group::saveComponent(int id, QString fn, bool forcename) {
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

  if (forcename || object(id).asGroup().attributes[Attribute::Footprint]=="")
    object(id).asGroup().attributes[Attribute::Footprint]
      = QFileInfo(fn).completeBaseName();

  QXmlStreamWriter sw(&file);
  sw.setAutoFormatting(true);
  sw.setAutoFormattingIndent(2);
  
  QByteArray svg = ORenderer::objectToSvg(obj,
                                          Dim::fromMils(50),
					  Dim::fromMils(300),
                                          10);
  QBuffer svgbuf(&svg);
  svgbuf.open(QBuffer::ReadOnly);
  QXmlStreamReader sr(&svgbuf);

  while (!sr.atEnd()) {
    sr.readNext();
    if (sr.isStartElement() && sr.name()=="svg") {
      sw.writeStartElement("svg");
      sw.writeAttributes(sr.attributes());
      for (auto ns: sr.namespaceDeclarations())
	sw.writeNamespace(ns.namespaceUri().toString(),
                          ns.prefix().toString());

      sw.writeNamespace("http://www.danielwagenaar.net/cpcb-ns.html", "cpcb");
      sw.writeStartElement("cpcb:part");

      Group const &grp(obj.asGroup());
      
      for (Attribute attr: grp.attributes.keys())
        sw.writeAttribute(xmlnames[attr], grp.attributes[attr]);
      sw.writeDefaultNamespace("http://www.danielwagenaar.net/cpcb-ns.html");
  
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
	if (o.asGroup().attributes[Group::Attribute::Footprint]=="")
	  o.asGroup().attributes[Group::Attribute::Footprint] = pkg;
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
      qDebug() << "Missing group in gr" << tid << t.object(tid).asText().text;
    if (tid<=0)
      qDebug() << "Missing ref text in gr" << gid << t.object(gid).asGroup().ref;
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
  for (auto it=xmlnames.begin(); it!=xmlnames.end(); ++it) 
    if (a.hasAttribute(it.value()))
      t.attributes[it.key()] = a.value(it.value()).toString();
  t.d->nominalrotation = a.value("nomrot").toInt();
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
  QStringList attribs;
  for (auto attr: t.attributes.keys())
    attribs << xmlnames[attr] + "=" + t.attributes[attr];
  d << "Group(" << t.ref << "["+attribs.join(", ")+"]" << t.d->nominalrotation;
  for (Object const &o: t.d->obj)
    d << "    " << o << "\n";
  d << ")";
  return d;
}

QList<NodeID> Group::findNodesByName(Nodename name) const {
  QList<NodeID> res;
  for (int id: d->obj.keys()) {
    Object const &obj(d->obj[id]);
    if (obj.isGroup() && res.isEmpty()) {
      Group const &grp(obj.asGroup());
      if (grp.ref.isEmpty()) {
	QList<NodeID> nids = grp.findNodesByName(name);
        for (NodeID &nid: nids)
	  nid.push_front(id);
        if (!nids.isEmpty())
          return nids;
      } else if (grp.ref==name.component()) {
	Nodename subn("", name.pin());
	QList<NodeID> nids = grp.findNodesByName(subn);
        for (NodeID &nid: nids)
	  nid.push_front(id);
	if (!nids.isEmpty()) 
	  return nids;
	}
    } else if (obj.isPad() || obj.isHole()) {
      Nodename n1("", obj.isPad() ? obj.asPad().ref : obj.asHole().ref);
      if (name.matches(n1)) {
	NodeID nid;
	nid << id;
        res << nid;
      }
    }
  }

  return res;
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

QSet<QString> Group::duplicatedGroupRefs() const {
  QSet<QString> seen;
  QSet<QString> dups;
  for (int id: keys()) {
    Object const &obj = object(id);
    if (obj.type()==Object::Type::Group) {
      QString ref = obj.asGroup().ref;
      if (seen.contains(ref))
        dups.insert(ref);
      seen.insert(ref);
    }
  }
  dups.remove("");
  return dups;
}  

QSet<QString> Group::badlyNamedGroupRefs() const {
  QSet<QString> refs;
  for (int id: keys()) {
    Object const &obj = object(id);
    if (obj.type()==Object::Type::Group) {
      QString ref = obj.asGroup().ref;
      if (ref.endsWith("?"))
        refs.insert(ref);
    }
  }
  return refs;
}  

QSet<QString> Group::duplicatedPinRefs() const {
  QSet<QString> seen;
  QSet<QString> dups;
  for (int id: keys()) {
    Object const &obj = object(id);
    QString ref = "";
    if (obj.type()==Object::Type::Hole)
      ref = obj.asHole().ref;
    else if (obj.type()==Object::Type::Pad)
      ref =  obj.asPad().ref;
    if (seen.contains(ref))
      dups.insert(ref);
    seen.insert(ref);
  }
  dups.remove("");
  return dups;
}

QMap<QString, QSet<QString>> Group::groupsWithDuplicatedPins() const {
  QMap<QString, QSet<QString>> grps;
  for (int id: keys()) {
    Object const &obj = object(id);
    if (obj.type()==Object::Type::Group) {
      QString ref = obj.asGroup().ref;
      QSet<QString> dups = obj.asGroup().duplicatedPinRefs();
      if (!dups.isEmpty())
        grps[ref] = dups;
    }
  }
  return grps;
}

static QString altRef(QString ref, QSet<QString> const &set) {
  if (ref.isEmpty())
    return "X?";
  static QRegularExpression re("^([^0-9?]+)([0-9?]*)([^0-9?]*)$");
  auto mtch = re.match(ref);
  if (!mtch.hasMatch())
    return ref; // !???
  QString pfx = mtch.captured(1);
  int num = mtch.captured(2).toInt() + 1;
  QString sfx = mtch.captured(3);
  auto inject = [pfx, sfx](int num) {
    return QString("%1%2%3").arg(pfx).arg(num).arg(sfx);
  };
  while (set.contains(inject(num)))
    ++num;
  return inject(num);
}

QSet<int> Group::merge(Group const &g) {
  QSet<QString> refs = immediateRefs();

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
      if (refs.contains(pad.ref))
        pad.ref = ::altRef(pad.ref, refs);
      refs << pad.ref;
    } break;
    case Object::Type::Hole: {
      Hole &hole(object(idmap[id]).asHole());
      if (refs.contains(hole.ref))
	hole.ref = ::altRef(hole.ref, refs);
      refs << hole.ref;
    } break;
    case Object::Type::Group: {
      Group &group(object(idmap[id]).asGroup());
      if (refs.contains(group.ref))
	group.ref = ::altRef(group.ref, refs);
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

bool Group::adjustViasAroundTrace(int traceid, Layer newlayer) {
  bool acted = false;
  Trace const &tr(object(traceid).asTrace());

  if (tr.layer==newlayer || !layerIsCopper(tr.layer))
    return false;
  
  // first, see if there are already vias at p1 or p2
  auto via_at = [this](Point const &p) {
    for (int id: d->obj.keys()) {
      Object const &obj(object(id));
      if (obj.isHole() && obj.asHole().p==p && obj.asHole().via)
	return id;
    }
    return -1;
  };
  int via_at_p1 = via_at(tr.p1);
  int via_at_p2 = via_at(tr.p2);

  // Also, see if there are already holes at p1 or p2 (incl. vias)
  auto hole_at = [this](Point const &p) {
    for (int id: d->obj.keys()) {
      Object const &obj(object(id));
      if (obj.isHole())
	if (obj.asHole().p.distance(p) < obj.asHole().od/2)
	  return id;
    }
    return -1;
  };
  int hole_at_p1 = hole_at(tr.p1);
  int hole_at_p2 = hole_at(tr.p2);

  // find other traces linked at those points
  auto traces_at = [this, traceid](Point const &p, int excl) {
    QList<int> res;
    for (int id: d->obj.keys()) {
      if (id==excl)
	continue;
      Object const &obj(object(id));
      if (obj.isTrace() && (obj.asTrace().p1==p || obj.asTrace().p2==p))
	res << id;
    }
    return res;
  };
  QList<int> trcs_at_p1 = traces_at(tr.p1, traceid);
  QList<int> trcs_at_p2 = traces_at(tr.p2, traceid);

  // find out which layers are involved
  auto layers_for = [this](QList<int> const &trcids) {
    QSet<Layer> res;
    for (int id: trcids)
      res |= object(id).asTrace().layer;
    return res;
  };
  QSet<Layer> layers_at_p1 = layers_for(trcs_at_p1);
  QSet<Layer> layers_at_p2 = layers_for(trcs_at_p2);
  
  // remove vias if they will serve no other purpose
  if (via_at_p1>0
      && (layers_at_p1.isEmpty()
	  || (layers_at_p1.size()==1 && layers_at_p1.contains(newlayer)))) {
    remove(via_at_p1);
    acted = true;
  }

  if (via_at_p2>0
      && (layers_at_p2.isEmpty()
	  || (layers_at_p2.size()==1 && layers_at_p2.contains(newlayer)))) {
    remove(via_at_p2);
    acted = true;
  }

  if (!layerIsCopper(newlayer))
    return acted;
  
  // add vias if necessary
  auto insert_via_at = [this](Point p, Dim w) {
    Hole h;
    h.via = true;
    h.p = p;
    Dim id = 0.5*w;
    Dim minID(Dim::fromMM(.3));
    h.id = max(id, minID);
    Dim od = w;
    Dim minOD = h.id + Dim::fromMM(.3);
    h.od = max(od, minOD);
    insert(Object(h));
  };
  if (hole_at_p1<0 && layers_at_p1.contains(tr.layer)) {
    insert_via_at(tr.p1, tr.width);
    acted = true;
  }
  if (hole_at_p2<0 && layers_at_p2.contains(tr.layer)) {
    insert_via_at(tr.p2, tr.width);
    acted = true;
  }
  return acted;
}

Group Group::subset(QSet<int> selection) const {
  Group g;
  QMap<int, int> idmap;
  for (int id: selection) 
    idmap[id] = g.insert(object(id));

  for (int id: selection) {
    if (object(id).isText()) {
      Text &txt(g.object(idmap[id]).asText());
      if (txt.groupAffiliation()>0
          && !selection.contains(txt.groupAffiliation())) {
        // orphaned ref text
        txt.setGroupAffiliation(0);
      }
    }
  }
  for (int id: selection) {
    if (object(id).isGroup()) {
      int gid = idmap[id];
      Group &grp(g.object(gid).asGroup());
      if (grp.refTextId()>0) {
	int tid = idmap[grp.refTextId()];
	grp.setRefTextId(tid);
        if (tid) {
          Text &txt(g.object(tid).asText());
          txt.setGroupAffiliation(gid);
        }
      }
    }
  }
  return g;
}

static int normalizedrotation(int rot) {
  rot /= 90;
  rot &= 3;
  return 90*rot;
}
  
int Group::nominalRotation() const {
  return normalizedrotation(d->nominalrotation);
}

void Group::setNominalRotation(int degccw) {
  d->nominalrotation = normalizedrotation(degccw);
}

bool Group::hasHoles() const {
  for (int id: keys()) 
    if (object(id).isHole() && object(id).asHole().ref!="")
      return true;
  return false;
}
