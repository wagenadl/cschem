// Element.h

#ifndef ELEMENT_H

#define ELEMENT_H

#include <QPoint>
#include <QXmlStreamReader>
#include <QDebug>

class Rotation {
public:
  Rotation(int r=0): r(r&3) { }
  Rotation &operator+=(int dr) { r=(r+dr)&3; return *this; }
  operator int() const { return r; }
private:
  int r;
};

class Element {
public:
  enum class Type {
    Invalid,
    Component,
    Port,
    Junction,
  };
  struct Info {
    QString vendor;
    QString partno;
    QString package;
    QString notes;
  };
public:
  Element();
  static Element junction(QPoint);
  static Element component(QString type, QPoint);
  static Element port(QString type, QPoint);
public:
  QString report() const;
public:
  bool isValid() const;
  QString tag() const; // "component"/"port"/"junction"
  QString symbol() const; // e.g., "part:passive:resistor"
  Element translated(QPoint delta) const;
public:
  void translate(QPoint delta);
  void autoSetVisibility();
  void copyAnnotationsFrom(Element const &);
public:
  Element::Type type;
  QPoint position;
  QString subtype;
  QString value;
  QString name;
  int id;
  Rotation rotation;
  bool flipped;
  Element::Info info;
  QPoint valuePosition;
  QPoint namePosition;
  bool valueVisible;
  bool nameVisible;
};

QXmlStreamWriter &operator<<(QXmlStreamWriter &, Element const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Element &);
QDebug &operator<<(QDebug &, Element const &);

#endif
