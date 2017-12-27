// Container.h

#ifndef CONTAINER_H

#define CONTAINER_H

#include <QSharedData>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QList>

class Container {
public:
  Container();
  Container(Container const &);
  Container(QXmlStreamReader &src);
  Container &operator=(Container const &);
  ~Container();
public:
  int id() const;
  QString name() const;
  QString type() const;
  QList<int> const &components() const;
public:
  void setId(int);
  void setName(QString);
  QList<int> &components();
  void setComponents(QList<int> const &);
  void setType(QString);
private:
  QSharedDataPointer<class ContainerData> d;
};

QXmlStreamWriter &operator<<(QXmlStreamWriter &, Container const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Container &);


#endif
