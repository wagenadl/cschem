// PickNPlace.cpp

#include "PickNPlace.h"
#include "Group.h"
#include "Object.h"
#include "data/CSV.h"
#include <QFile>
#include "BOMTable.h"
#include <QTextStream>
#include "circuit/PartNumbering.h"
#include <algorithm>

PNPLine::PNPLine() {
  valid = false;
}

PNPLine::PNPLine(Group const &g, Dim boardheight) {
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
  orient = (g.nominalRotation() % 360 + 360) % 360;
  footprint = g.attributes.value(Group::Attribute::Footprint);
  comment = g.attributes.value(Group::Attribute::Notes);
  if (footprint=="")
    valid = false;

  center.y = boardheight - center.y;
  pin1.y = boardheight - pin1.y;
  bbox.top = boardheight - bbox.top - bbox.height; // is that right?
  
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

PickNPlace::PickNPlace(Group const &root, Dim boardheight, PickNPlace::Scope scope) {
  for (int id: root.keys()) {
    Object const &obj = root.object(id);
    if (!obj.isGroup())
      continue;
    Group const &g(obj.asGroup());
    if (scope==Scope::SMTAndThruHole || !g.hasHoles()) {
      PNPLine line(g, boardheight);
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
  std::sort(list.begin(), list.end(),
            [](QStringList a, QStringList b) {
              return PartNumbering::lessThan(a[0], b[0]); });
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

  
QList<PNPLine> const &PickNPlace::placed() const {
  return lines;
}
