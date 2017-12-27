// Port.h

#ifndef PORT_H

#define PORT_H

#include <QSharedData>
#include <QXmlStreamReader>

class Port {
public:
  Port();
  Port(Port const &);
  Port(QXmlStreamReader &src);
  Port &operator=(Port const &);
  ~Port();
public:
  QPoint position() const;
  QString type() const;
  QString name() const;
  int id() const;
  int rotation() const;
public:
  void setPosition(QPoint);
  void setType(QString);
  void setName(QString);
  void setId(int);
  void setRotation(int);
private:
  QSharedDataPointer<class PortData> d;
};

QXmlStreamWriter &operator<<(QXmlStreamWriter &, Port const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Port &);


#endif
