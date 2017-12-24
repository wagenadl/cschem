// XmlElement.h

#ifndef XMLELEMENT_H

#define XMLELEMENT_H

#include <QXmlStreamReader>
#include <QList>
#include "XmlNode.h"

class XmlElement {
public:
  XmlElement(QXmlStreamReader &src);
  // src must point to startElement. On return, points to corresponding
  // endElement.
  ~XmlElement();
  bool isValid() const { return valid_; }
  QList<XmlNode> const &children() const { return children_; }
  QString qualifiedName() const { return qualifiedName_; }
  QXmlStreamAttributes attributes() const { return attributes_; }
  QXmlStreamNamespaceDeclarations namespaceDeclarations() const {
    return namespaceDeclarations_; }
  void write(QXmlStreamWriter &dst) const;
  void writeStartElement(QXmlStreamWriter &dst) const;
  void writeChildren(QXmlStreamWriter &dst) const;
  void writeEndElement(QXmlStreamWriter &dst) const;
private:
  bool valid_;
  QString qualifiedName_;
  QXmlStreamAttributes attributes_;
  QXmlStreamNamespaceDeclarations namespaceDeclarations_;
  QList<XmlNode> children_;
};

#endif
