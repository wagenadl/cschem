// PickNPlace.cpp

#include "PickNPlace.h"
#include "Group.h"
#include "Object.h"
#include "data/CSV.h"
#include <QFile>

PNPLine::PNPLine() {
  valid = false;
}

PNPLine::PNPLine(Group const &g) {
  valid = false;
  ref = g.ref;
  QStringList pins = g.pinNames();
  bbox = g.boundingRect();
  Rect pinbox = Rect();
  for (QString pin: g.pinNames()) {
    if (pin=="1" || pin.startsWith("1/") || pin.endsWith("/1")) {
      valid = true;
      pin1 = g.pinPosition(pin);
    }
    pinbox |= g.pinPosition(pin);
  }
  center = pinbox.center();
  orient = -(g.nominalRotation() % 360);
  footprint = g.attributes.value(Group::Attribute::Footprint);
  comment = g.attributes.value(Group::Attribute::Notes);
  if (footprint=="")
    valid = false;
}

void PNPLine::augment(Circuit const &circuit) {
  if (comment != "")
    return;
  int elt = circuit.elementByName(ref);
  if (elt<=0)
    return;
  comment = circuit.elements[elt].value;
  if (comment.startsWith('"') || comment.startsWith("“"))
    comment = "";
  else if (comment=="" && !circuit.elements[elt].subtype.contains(":"))
    comment = circuit.elements[elt].subtype;
}

bool PNPLine::isValid() const {
  return valid;
}

QStringList PNPLine::toStringList() const {
  return QStringList{
    ref, footprint,
    dimToString(center.x), dimToString(center.y),
    dimToString(center.x), dimToString(center.y),
    dimToString(pin1.x), dimToString(pin1.y),
    "T", QString::number(orient),
    comment};
}

QStringList PNPLine::header() {
  static QStringList hdr{
    "Designator", "Footprint",
    "Mid X", "Mid Y",
    "Ref X", "Ref Y",
    "Pad X", "Pad Y",
    "Layer", "Rotation",
    "Comment",
  };
  return hdr;
}

QString PNPLine::dimToString(Dim x) {
  double mm = x.toMM();
  return QString("%1mm").arg(mm, 0, 'f', 2);
}

PickNPlace::PickNPlace() {
}

PickNPlace::PickNPlace(Group const &root, PickNPlace::Scope scope) {
  for (int id: root.keys()) {
    Object const &obj = root.object(id);
    if (!obj.isGroup())
      continue;
    Group const &g(obj.asGroup());
    if (scope==Scope::SMTOnly && g.hasHoles()) {
      unrefs << g.ref;
    } else {
      PNPLine line(g);
      if (line.isValid())
        lines << line;
      else
        unrefs << g.ref;
    }
  }    
}

void PickNPlace::augment(Circuit const &circuit) {
  for (PNPLine &line: lines)
    line.augment(circuit);
}

QList<QStringList> PickNPlace::toList() const {
  QList<QStringList> list;
  for (PNPLine const &line: lines)
    list << line.toStringList();
  return list;
}

bool PickNPlace::saveCSV(QString fn) const {
  QList<QStringList> list = toList();
  list.insert(0, PNPLine::header());
  QFile f(fn);
  if (f.open(QFile::WriteOnly)) {
    QTextStream(&f) << CSV::encode(list);
    return true;
  } else {
    return false;
  }
}

QStringList PickNPlace::placedRefs() const {
  QStringList refs;
  for (PNPLine const &line: lines)
    refs << line.ref;
  return refs;
}

QStringList PickNPlace::unplacedRefs() const {
  return unrefs;
}
