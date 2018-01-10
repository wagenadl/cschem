// Parts.h

#ifndef PARTS_H

#define PARTS_H

#include <QSharedData>
#include <QXmlStreamReader>
#include "Container.h"
#include "PartInfo.h"
#include <QMap>

class Parts {
public:
  Parts();
  Parts(Parts const &);
  Parts(QXmlStreamReader &src);
  Parts &operator=(Parts const &);
  ~Parts();
public:
  QMap<int, class Container> const &containers() const;
  QMap<int, class PartInfo> const &infos() const;
  bool isEmpty() const;
  void insert(PartInfo const &);
  void insert(Container const &);
  void removeInfo(int id);
  void removeContainer(int id);
  void renumber(QMap<int, int>);
private:
  QSharedDataPointer<class PartsData> d;
};

QXmlStreamWriter &operator<<(QXmlStreamWriter &, Parts const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Parts &);


#endif
