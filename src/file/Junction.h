// Junction.h

#ifndef JUNCTION_H

#define JUNCTION_H

#include <QSharedData>
#include <QXmlStreamReader>

class Junction {
public:
  Junction();
  Junction(Junction const &);
  Junction(QXmlStreamReader &src);
  Junction &operator=(Junction const &);
  ~Junction();
public:
  QPoint position() const;
  int id() const;
public:
  void setPosition(QPoint);
  void setId(int);
private:
  QSharedDataPointer<class JunctionData> d;
};

QXmlStreamWriter &operator<<(QXmlStreamWriter &, Junction const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Junction &);


#endif
