// Element.h

#ifndef ELEMENT_H

#define ELEMENT_H

#include <QSharedData>
#include <QXmlStreamReader>

class Element {
public:
  enum class Type {
    Invalid,
    Component,
    Port,
    Junction
  };
public:
  static Element junction(QPoint);
  static Element component(QString type, QPoint);
  static Element port(QString type, QPoint);
public:
  Element();
  Element(Element const &);
  Element(QXmlStreamReader &src);
  Element &operator=(Element const &);
  ~Element();
  QString report() const;
public:
  QPoint position() const;
  Type type() const;
  bool isValid() const;
  QString tag() const; // component/port/junction
  QString subtype() const;
  QString symbol() const;
  QString value() const;
  QString name() const;
  QString label() const;
  int id() const;
  int rotation() const;
  Element translated(QPoint delta) const;
public:
  void setPosition(QPoint);
  void setSubtype(QString);
  void setValue(QString);
  void setName(QString);
  void setLabel(QString);
  void setId(int); // should only be used for well-controlled renumber op
  void setRotation(int);
  void translate(QPoint delta);
private:
  QSharedDataPointer<class ElementData> d;
};

QXmlStreamWriter &operator<<(QXmlStreamWriter &, Element const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Element &);


#endif
