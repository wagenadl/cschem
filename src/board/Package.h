// Package.h

#ifndef PACKAGE_H

#define PACKAGE_H

#include "circuit/Element.h"

class Package: public Element {
public:
  enum class Type {
    Invalid,
    Component,
    Hole,
    Via,
    Junction
  };
  enum class Layer {
    TopSilk,
    BottomSilk,
    Top,
    Bottom,
    Inner1,
    Inner2,
    Inner3,
    Inner4
  };
public:
  static Package junction(QPoint, Layer);
  static Package component(QString type, QPoint, Layer);
  static Package via(QString type, QPoint);
  static Package hole(QString type, QPoint);
public:
  Package();
  Package(Package const &);
  Package(QXmlStreamReader &src);
  Package &operator=(Package const &);
  ~Package();
  QString report() const;
public:
  Type type() const;
  Layer layer() const;
  bool isFlipped() const;
  bool isValid() const;
public:
  void setLayer(Layer);
  void setFlipped(bool);
protected:
  void writeAttributes(QXmlStreamWriter &) const override;
  void readAttributes(QXmlStreamReader &) override;
  friend QXmlStreamWriter &operator<<(QXmlStreamWriter &, Package const &);
  friend QXmlStreamReader &operator>>(QXmlStreamReader &, Package &);
private:
  QSharedDataPointer<class PackageData> d;
};

QXmlStreamWriter &operator<<(QXmlStreamWriter &, Package const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Package &);

#endif
