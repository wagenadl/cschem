// reftest.cpp

#include <QVector>
#include <QDebug>
#include <QList>
#include <QMap>

int main() {
  QList<QVector<QSet<int>>> a;
  QSet<int> set; set << 1;
  a << QVector<QSet<int>>();
  a[0] << set;
  set << 2;
  a[0] << set;
  set << 3;
  QVector<QSet<int>> &ar(a[0]);
  QSet<int> &aa(a[0][1]);
  QList<QVector<QSet<int>>> b(a);
  aa << 4;
  ar[0] << 5;
  qDebug() << "a" << a;
  qDebug() << "b" << b;

  QMap<int, QString> map;
  map[0] = "zero";
  map[1] = "one";
  QString &s(map[0]);
  qDebug() << s;
  map.remove(0);
  qDebug() << map;
  qDebug() << s;
  return 0;
}

