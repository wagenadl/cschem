// SafeMap.h

#ifndef SAFEMAP_H

#define SAFEMAP_H

#include <QDebug>
#include <QMap>

void safemap_illegal();

template <class Key, class Value> class SafeMap: public QMap<Key, Value> {
  /* A SafeMap is a map that does not automatically insert values when
   accessed. That is, if M is a SafeMap that does contain X but not Y,
   then M[X] behaves just as if M were a regular QMap, but M[Y] is special:
   M[Y] = A does not work. B = M[Y] gives a warning. */
public:
  Value const &operator[](Key const &key) const {
    static Value vnul;
    auto it = QMap<Key,Value>::find(key);
    if (it==QMap<Key,Value>::end())
      return vnul;
    else
      return *it;
  }
  Value &operator[](Key const &key) {
    static Value vnul;
    auto it = QMap<Key,Value>::find(key);
    if (it==QMap<Key,Value>::end()) {
      qDebug() << "Illegal access" << key;
      safemap_illegal();
      vnul = Value();
      return vnul;
    } else {
      return *it;
    }
  }
};

#endif
