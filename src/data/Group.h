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
  /* Invariant: A group may not be empty when placed inside a Layout.
     Empty groups are used to reflect "invalid" status.
  */
public:
  QString ref;
  QString notes;
  QString pkg; // name of component file
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
  QList<int> keys() const;
  bool isEmpty() const;
  Group const &subgroup(QList<int> path) const; // follows breadcrumbs
  Group &subgroup(QList<int> path);
  // Caution: Do NOT change the empty group that may be returned.
  Rect boundingRect() const; // relative to parent
  bool touches(Point p, Dim mrg=Dim()) const;
  QList<int> objectsAt(Point p, Dim mrg=Dim()) const; // p is relative to parent
  // only direct children are returned
  NodeID nodeAt(Point p, Dim mrg=Dim()) const;
  // even inside subgroups; return is crumbs from this group
  NodeID findNodeByName(Nodename name) const;
  QString humanName(NodeID const &) const;
  QStringList nodePath(NodeID const &) const; // refs for each level
  Nodename nodeName(NodeID const &) const;
  int formSubgroup(QSet<int> const &);
  QSet<int> dissolveSubgroup(int);
  int ensureRefText(int); // ensure that subgroup has reftext object
  void rotateCCW(Point p);
  void rotateCW(Point p);
  void flipLeftRight(Dim x);
  void flipUpDown(Dim y);
  void translate(Point p);
  Group translated(Point p) const;
  int refTextId() const; // ID (in parent group) of the text object
  // that represents our Ref., or 0 if none.
  void setRefTextId(int);
  bool saveComponent(int id, QString fn); // id must be a subgroup
  int insertComponent(QString fn);
  QSet<Point> points() const;
  QSet<Point> points(Layer) const;
  QStringList pinNames() const; // immediate children
  int pinID(QString) const; // child id for named pin or -1
  QString pinName(int) const; // ref for given object or "" if not Hole/Pad
  Point pinPosition(QString name) const;
  Point anchor() const; // first named pin
  void insertSegmentedTrace(class Trace const &t);
  Point intersectionWith(class Trace const &t, int *idp=0) const;
  void setPinRef(int id, QString ref);
  /* Sets reference for object ID (which must be Hole or Pad). Also
     steals that reference from other objects. E.g., if there is a pin
     named "1/D" and another pin gets named "2/D", then the first pin
     loses the "/D" suffix.
   */
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
