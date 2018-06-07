// Object.h

#ifndef OBJECT_H

#define OBJECT_H

#include "Hole.h"
#include "Pad.h"
#include "Trace.h"
#include "Text.h"
#include "Group.h"
#include <QXmlStreamReader>
#include <QDebug>
#include <QSharedData>

class Object {
public:
  enum class Type {
    Null,
    Hole,
    Pad,
    Text,
    Trace,
    Group,
  };
public:
  explicit Object(Hole const &);
  explicit Object(Pad const &);
  explicit Object(Trace const &);
  explicit Object(Text const &);
  explicit Object(Group const &);
  Object();
  ~Object();
  Object(Object const &);
  Object &operator=(Object const &);
  bool isNull() const;
  bool isHole() const;
  bool isPad() const;
  bool isTrace() const;
  bool isText() const;
  bool isGroup() const;  
  Hole const &asHole() const;
  Hole &asHole();
  // Both const and nonconst versions ASSERT correct type.
  // Use isXXX first to check!
  Pad const &asPad() const;
  Pad &asPad();
  Trace const &asTrace() const;
  Trace &asTrace();
  Text const &asText() const;
  Text &asText();
  Group const &asGroup() const;
  Group &asGroup();
  Type type() const;
  bool touches(Point p, Dim mrg=Dim()) const;
  Rect boundingRect() const;
  Layer layer() const;
  void translate(Point const &);
private:
  QSharedDataPointer<class OData> d;
};

QDebug operator<<(QDebug, Object const &);
QXmlStreamWriter &operator<<(QXmlStreamWriter &, Object const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Object &);

#endif
