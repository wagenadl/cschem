// PartsList.cpp

#include "PartsList.h"
#include <QDebug>
#include "PinID.h"

class PartsListData: public QSharedData {
public:
  PartsListData(bool valid=true): valid(valid) { }
public:
  QMap<int, Part> parts;
  bool valid;
};  

PartsList::PartsList() {
  d = new PartsListData;
}

PartsList::PartsList(PartsList const &o) {
  d = o.d;
}


PartsList &PartsList::operator=(PartsList const &o) {
  d = o.d;
  return *this;
}

PartsList::~PartsList() {
}

QMap<int, class Part> const &PartsList::parts() const {
  return d->parts;
}

QXmlStreamReader &operator>>(QXmlStreamReader &sr, PartsList &c) {
  while (!sr.atEnd()) {
    sr.readNext();
    if (sr.isStartPart()) {
      auto n = sr.name();
      if (n=="part") {
	Part elt;
	sr >> elt;
	c.d->parts[elt.id()] = elt;
      } else {
	qDebug() << "Unexpected element in PartsList: " << sr.name();
	c.d->valid = false;
      }
    } else if (sr.isEndElement()) {
      break;
    } else if (sr.isCharacters() && sr.isWhitespace()) {
    } else if (sr.isComment()) {
    } else {
      qDebug() << "Unexpected entity in PartsList: " << sr.tokenType();
      c.d->valid = false;
    }
  }
  // now at end of PartsList element
  return sr;
}
  
QXmlStreamWriter &operator<<(QXmlStreamWriter &sr, PartsList const &c) {
  sr.writeStartElement("parts");
  for (auto const &c: c.parts())
    sr << c;
  sr.writeEndElement();
  return sr;
}

void PartsList::insert(Part const &e) {
  d.detach();
  d->parts[e.id()] = e;
}

void PartsList::remove(int id) {
  if (d->parts.contains(id)) {
    d.detach();
    d->parts.remove(id);
  }
}

Part const &PartsList::part(int id) const {
  static Part nullpart;
  auto it(d->parts.find(id));
  return it == d->parts.end() ? nullpart : *it;
}

int PartsList::renumber(int start) {
  QList<Part> elts = d->parts.values();
  d->parts.clear();

  for (Part elt: elts) {
    int oldid = elt.id();
    elt.setId(start);
    d->parts.remove(oldid);
    d->parts[start] = elt;
    start ++;
  }
  return start - 1;
}

bool PartsList::isEmpty() const {
  return parts().isEmpty();
}

bool PartsList::isValid() const {
  return d->valid;
}

int PartsList::maxId() const {
  int mx = 0;
  for (int id: d->parts.keys())
    if (id>mx)
      mx = id;
  return mx;
}

PartsList &PartsList::operator+=(PartsList const &o) {
  for (auto elt: o.parts())
    d->parts[elt.id()] = elt;
  return *this;
}

Part const &PartsList::findPart(QString name) const {
  static Part nullpart;
  QString basename = name;
  int idx = basename.indexOf(".");
  if (idx>0)
    basename = basename.left(idx);
  // etc.
}
