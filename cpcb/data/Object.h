// Object.h

#ifndef OBJECT_H

#define OBJECT_H

#include "Hole.h"
#include "NPHole.h"
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
    NPHole,
  };
public:
  explicit Object(Hole const &);
  explicit Object(Pad const &);
  explicit Object(Arc const &);
  explicit Object(Trace const &);
  explicit Object(Text const &);
  explicit Object(Group const &);
  explicit Object(FilledPlane const &);
  explicit Object(NPHole const &);
  Object();
  ~Object();
  Object(Object const &);
  Object &operator=(Object const &);
  bool isNull() const;
  bool isHole() const;
  bool isNPHole() const;
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
  NPHole const &asNPHole() const;
  NPHole &asNPHole();
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
  Rect boundingRect() const;
  Layer layer() const;
  void translate(Point const &);
  Object translated(Point const &) const;
  void rotateCCW(Point const &, bool nottext=false);
  void rotateCW(Point const &, bool nottext=false);
  void freeRotate(FreeRotation const &degCW, Point const &);
  void flipLeftRight(Dim x, bool nottext=false);
  void flipUpDown(Dim y, bool nottext=false);
  QSet<Point> pinPoints() const; // recursive
  QSet<Point> pinPoints(Layer l) const;
  QSet<Point> allPoints() const; // incl. traces and arcs
  QSet<Point> allPoints(Layer) const;
  QSet<Point> altCoords() const;
private:
  QSharedDataPointer<class OData> d;
};

QDebug operator<<(QDebug, Object const &);
QXmlStreamWriter &operator<<(QXmlStreamWriter &, Object const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Object &);

#endif
