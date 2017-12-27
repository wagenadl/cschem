// Parts.h

#ifndef PARTS_H

#define PARTS_H

#include <QSharedData>
#include <QXmlStreamReader>
#include "Container.h"
#include "Package.h"
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
  QMap<int, class Container> &containers();
  QMap<int, class Package> const &packages() const;
  QMap<int, class Package> &packages();
private:
  QSharedDataPointer<class PartsData> d;
};

QXmlStreamWriter &operator<<(QXmlStreamWriter &, Parts const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Parts &);


#endif
