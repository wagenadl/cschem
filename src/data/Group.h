// Group.h

#ifndef GROUP_H

#define GROUP_H

#include <QList>
#include <QMap>
#include <QDebug>
#include <QXmlStreamReader>
#include <QSharedData>

class Group {
public:
  Group();
  ~Group();
  Group(Group const &);
  Group &operator=(Group const &);
  int insert(class Object const &); // assigns ID
  void remove(int);
  bool contains(int) const;
  Object const &object(int) const;
  QList<int> keys() const;
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
