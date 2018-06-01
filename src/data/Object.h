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
    Invalid,
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
  bool isHole() const;
  bool isPad() const;
  bool isTrace() const;
  bool isText() const;
  bool isGroup() const;  
  Hole toHole() const;
  Pad toPad() const;
  Trace toTrace() const;
  Text toText() const;
  Group toGroup() const;
  Type type() const;
private:
  QSharedDataPointer<class OData> d;
};

QDebug operator<<(QDebug, Object const &);
QXmlStreamWriter &operator<<(QXmlStreamWriter &, Object const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Object &);

#endif
