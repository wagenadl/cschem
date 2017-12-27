// Component.h

#ifndef COMPONENT_H

#define COMPONENT_H

#include <QSharedData>
#include <QXmlStreamReader>

class Component {
public:
  Component();
  Component(Component const &);
  Component(QXmlStreamReader &src);
  Component &operator=(Component const &);
  ~Component();
public:
  QPoint position() const;
  QString type() const;
  QString value() const;
  QString name() const;
  QString label() const;
  int id() const;
  int rotation() const;
public:
  void setPosition(QPoint);
  void setType(QString);
  void setValue(QString);
  void setName(QString);
  void setLabel(QString);
  void setId(int);
  void setRotation(int);
private:
  QSharedDataPointer<class ComponentData> d;
};

QXmlStreamWriter &operator<<(QXmlStreamWriter &, Component const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Component &);


#endif
