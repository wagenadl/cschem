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
  struct Info {
    QString vendor;
    QString partno;
    QString notes;
  };
public:
  static Element junction(QPoint);
  static Element component(QString type, QPoint);
  static Element port(QString type, QPoint);
public:
  Element();
  Element(Element const &);
  Element &operator=(Element const &);
  ~Element();
  QString report() const;
public:
  QPoint position() const;
  Type type() const;
  Info info() const;
  bool isValid() const;
  QString tag() const; // "component"/"port"/"junction"
  QString subtype() const; // e.g., "passive:resistor"
  QString symbol() const; // e.g., "part:passive:resistor"
  QString value() const; // e.g., "10 pF" or "INA111"
  QString name() const; // e.g., "R1"
  QPoint valuePos() const;
  QPoint namePos() const;
  bool isValueVisible() const;
  bool isNameVisible() const;
  bool isVirtual() const; // an element is virtual if it
  // has "virtual" in its symbol name
  int id() const;
  int rotation() const;
  bool isFlipped() const;
  Element translated(QPoint delta) const;
public:
  void setPosition(QPoint);
  void setSubtype(QString);
  void setInfo(Info const &);
  void setValue(QString);
  void setName(QString);
  void setValuePos(QPoint);
  void setNamePos(QPoint);
  void setValueVisible(bool);
  void setNameVisible(bool);
  void setId(int); // should only be used for well-controlled renumber op
  void setRotation(int);
  void setFlipped(bool);
  void translate(QPoint delta);
protected:
  friend QXmlStreamWriter &operator<<(QXmlStreamWriter &, Element const &);
  friend QXmlStreamReader &operator>>(QXmlStreamReader &, Element &);
  virtual void writeAttributes(QXmlStreamWriter &) const;
  virtual void readAttributes(QXmlStreamReader &);
private:
  QSharedDataPointer<class ElementData> d;
};

QXmlStreamWriter &operator<<(QXmlStreamWriter &, Element const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Element &);


#endif
