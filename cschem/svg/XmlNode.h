// XmlNode.h

#ifndef XMLNODE_H

#define XMLNODE_H

#include <QXmlStreamReader>
#include <QSharedData>

class XmlNodeData;

class XmlNode {
public:
  enum class Type {
    Invalid,
    Element,
    Text,
  };
public:
  XmlNode();
  XmlNode(QXmlStreamReader &src);
  XmlNode(XmlNode const &);
  XmlNode &operator=(XmlNode const &);
  ~XmlNode();
  Type type() const;
  class XmlElement const &element() const;
  class XmlElement &element();
  QString text() const;
  void setText(QString);
  void write(QXmlStreamWriter &writer) const;
  QString toString() const;
public:
  static XmlNode elementNode(XmlElement const &);
  static XmlNode textNode(QString str);
private:
  QSharedDataPointer<XmlNodeData> d;
};

#endif
