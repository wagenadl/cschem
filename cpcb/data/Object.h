// Object.h

#ifndef OBJECT_H

#define OBJECT_H

#include "Hole.h"
#include "Pad.h"
#include "Arc.h"
#include "Trace.h"
#include "Text.h"
#include "Group.h"
#include "FilledPlane.h"
#include <QXmlStreamReader>
#include <QDebug>
#include <QSharedData>

class Object {
public:
  enum class Type {
    Null,
    Hole,
    Pad,
    Arc,
    Text,
    Trace,
    Plane,
    Group,
  };
public:
  explicit Object(Hole const &);
  explicit Object(Pad const &);
  explicit Object(Arc const &);
  explicit Object(Trace const &);
  explicit Object(Text const &);
  explicit Object(Group const &);
  explicit Object(FilledPlane const &);
  Object();
  ~Object();
  Object(Object const &);
  Object &operator=(Object const &);
  bool isNull() const;
  bool isHole() const;
  bool isPad() const;
  bool isArc() const;
  bool isTrace() const;
  bool isText() const;
  bool isPlane() const;
  bool isGroup() const;  
  Hole const &asHole() const;
  Hole &asHole();
  // Both const and nonconst versions ASSERT correct type.
  // Use isXXX first to check!
  Pad const &asPad() const;
  Pad &asPad();
  Arc const &asArc() const;
  Arc &asArc();
  Trace const &asTrace() const;
  Trace &asTrace();
  Text const &asText() const;
  Text &asText();
  Group const &asGroup() const;
  Group &asGroup();
  FilledPlane const &asPlane() const;
  FilledPlane &asPlane();
  Type type() const;
  bool touches(Point p, Dim mrg=Dim()) const;
  /* Note: Touches returns false for filled planes, because they are
     not to be selected along with other stuff. */
  Rect boundingRect() const;
  Layer layer() const;
  void translate(Point const &);
  Object translated(Point const &) const;
  void rotateCCW(Point const &);
  void rotateCW(Point const &);
  void flipLeftRight(Dim x);
  void flipUpDown(Dim y);
  QSet<Point> pinPoints() const; // recursive
  QSet<Point> pinPoints(Layer l) const;
  QSet<Point> allPoints() const; // incl. traces and arcs
  QSet<Point> allPoints(Layer) const;
private:
  QSharedDataPointer<class OData> d;
};

QDebug operator<<(QDebug, Object const &);
QXmlStreamWriter &operator<<(QXmlStreamWriter &, Object const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Object &);

#endif
