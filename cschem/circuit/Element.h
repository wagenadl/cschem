// Element.h

#ifndef ELEMENT_H

#define ELEMENT_H

#include <QPoint>
#include <QXmlStreamReader>
#include <QDebug>
#include "PinID.h"

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
  bool isContainer() const;
  Element translated(QPoint delta) const;
  /* Following are convenience interfaces to the corresponding functions
     in PartNumbering. */
  QString prefix() const; // "R" part of "R3"
  int number() const;  ; // 3 part of "A3.2"; 0 if none
  int subNumber() const; // 2 part of "A3.2"; 0 if none
  QString cname() const; // "A3" part of "A3.2"
  QString csuffix() const; // ".2" part of "A3.2"
  bool isNameWellFormed() const;
  QString humanPinName(QString pin=PinID::NOPIN) const;
public:
  void translate(QPoint delta);
  void autoSetVisibility();
  void copyAnnotationsFrom(Element const &);
  bool operator==(Element const &) const;
  bool operator!=(Element const &) const;
public:
  Element::Type type;
  QPoint position;
  QString subtype;
  QString value;
  QString name;
  QString notes;
  int id;
  Rotation rotation;
  bool flipped;
  QPoint valuePosition;
  QPoint namePosition;
  bool valueVisible;
  bool nameVisible;
};

QXmlStreamWriter &operator<<(QXmlStreamWriter &, Element const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Element &);
QDebug operator<<(QDebug, Element const &);

#endif
