// XmlNode.h

#ifndef XMLNODE_H

#define XMLNODE_H

#include <QXmlStreamReader>
#include <QSharedPointer>

class XmlNode {
public:
  enum class Type {
    Invalid,
    Element,
    Text,
  };
public:
  XmlNode(QXmlStreamReader &src);
  XmlNode(): type_(Type::Invalid) {}
  ~XmlNode();
  Type type() const { return type_; }
  class XmlElement *element() const;
  QString text() const { return text_; }
  void write(QXmlStreamWriter &writer) const;
  QString toString() const;
private:
  Type type_;
  QSharedPointer<class XmlElement> element_;
  QString text_;
};

#endif
