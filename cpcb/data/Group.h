// Group.h

#ifndef GROUP_H

#define GROUP_H

#include "Layer.h"
#include "Rect.h"
#include "NodeID.h"
#include "Nodename.h"
#include <QList>
#include <QMap>
#include <QDebug>
#include <QXmlStreamReader>
#include <QSharedData>

class Group {
  /* Empty groups are not saved, even if they do have a ref or notes. */
public:
  enum class Attribute {
    Footprint,
    Manufacturer,
    PartNo,
    Vendor,
    CatNo,
    Notes,
  };
public:
  QString ref;
  QMap<Attribute, QString> attributes;
public:
  Group();
  ~Group();
  Group(Group const &);
  Group &operator=(Group const &);
  int insert(class Object const &); // assigns ID
  void remove(int);
  bool contains(int) const;
  Object const &object(int) const;
  Object &object(int);
  Object const &object(NodeID const &) const;
  Object &object(NodeID const &);
  QList<int> keys() const;
  QList<NodeID> allPins() const; // recursively
  bool isEmpty() const;
  Group const &subgroup(NodeID path) const; // follows breadcrumbs
  Group &subgroup(NodeID path);
  // Caution: Do NOT change the empty group that may be returned.
  Group const &parentOf(NodeID) const;
  Group &parentOf(NodeID);
  Group const &subByRef(QString ref) const; // empty if none
  Rect boundingRect() const; // relative to parent
  bool touches(Point p, Dim mrg=Dim()) const;
  QList<int> objectsAt(Point p, Dim mrg=Dim()) const; // p is relative to parent
  // only direct children are returned
  NodeID nodeAt(Point p, Dim mrg=Dim(),
                Layer l=Layer::Invalid, bool notrace=false) const;
  // even inside subgroups; return is crumbs from this group
  // optional layer limits pads to given layer
  NodeID findNodeByName(Nodename name) const;
  QString humanName(NodeID const &) const;
  QStringList nodePath(NodeID const &) const; // refs for each level
  Nodename nodeName(NodeID const &) const;
  int formSubgroup(QSet<int> const &);
  QSet<int> dissolveSubgroup(int);
  int ensureRefText(int); // ensure that subgroup has reftext object
  void rotateCCW(Point p);
  void rotateCW(Point p);
  void freeRotate(int degcw, Point const &p);
  int nominalRotation() const; // 0, 90, 180, or 260
  void setNominalRotation(int degccw);
  void flipLeftRight(Dim x);
  void flipUpDown(Dim y);
  void translate(Point p);
  Group translated(Point p) const;
  int refTextId() const; // ID (in parent group) of the text object
  // that represents our Ref., or 0 if none.
  void setRefTextId(int);
  bool saveComponent(int id, QString fn, bool forcename=false);
  // id must be a subgroup
  /* saveComponent is not const, because it sets the subgroup's pkg
     name if previously empty or if forcename is given as true.
  */
  int insertComponent(QString fn);
  QSet<Point> pinPoints() const; // hole and pad centers of immediate children
  QSet<Point> pinPoints(Layer) const; // same, limited to given layer
  QSet<Point> allPoints() const; // holes, pads, elbows
  QSet<Point> allPoints(Layer) const;
  QSet<Point> altCoords() const; // includes text anchors and planes
  QStringList pinNames() const; // immediate children
  QString pinName(int) const; // ref for given object or "" if not Hole/Pad
  Point pinPosition(QString name) const;
  Point anchor() const; // first named pin
  void setPinRef(int id, QString ref);
  /* Sets reference for object ID (which must be Hole or Pad). Also
     steals that reference from other objects. E.g., if there is a pin
     named "1/D" and another pin gets named "2/D", then the first pin
     loses the "/D" suffix.
   */
  QSet<QString> immediateRefs() const;
  // names of holes, pads, and groups (not recursively)
  QSet<int> merge(Group const &g); // return is ids
  bool adjustViasAroundTrace(int traceid, Layer newlayer); // true if chgs
  Group subset(QSet<int> ids) const; // construct a subset group
  QSet<QString> duplicatedGroupRefs() const; // refs that occur more than once
  QSet<QString> badlyNamedGroupRefs() const; // names that end in "?"
  QSet<QString> duplicatedPinRefs() const; // refs that occur more than once
  QMap<QString, QSet<QString>> groupsWithDuplicatedPins() const;
  bool hasHoles() const;
private:
  QSharedDataPointer<class GData> d;
  friend QDebug operator<<(QDebug, Group const &);  
  friend QXmlStreamWriter &operator<<(QXmlStreamWriter &, Group const &);
  friend QXmlStreamReader &operator>>(QXmlStreamReader &, Group &);
};

QDebug operator<<(QDebug, Group const &);  
QXmlStreamWriter &operator<<(QXmlStreamWriter &, Group const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Group &);

#endif
