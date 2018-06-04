// Group.h

#ifndef GROUP_H

#define GROUP_H

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
