// XmlElement.h

#ifndef XMLELEMENT_H

#define XMLELEMENT_H

#include <QSharedData>
#include <QXmlStreamReader>
#include <QList>
#include "XmlNode.h"

class XmlElementData;

class XmlElement {
public:
  XmlElement();
  XmlElement(QXmlStreamReader &src);
  XmlElement(XmlElement const &);
  XmlElement &operator=(XmlElement const &);
  // src must point to startElement. On return, points to corresponding
  // endElement.
  ~XmlElement();
  bool isValid() const;
  QList<XmlNode> const &children() const;
  QString qualifiedName() const;
  QXmlStreamAttributes attributes() const;
  QXmlStreamNamespaceDeclarations namespaceDeclarations() const;
  void write(QXmlStreamWriter &dst) const;
  void writeStartElement(QXmlStreamWriter &dst) const;
  void writeChildren(QXmlStreamWriter &dst) const;
  void writeEndElement(QXmlStreamWriter &dst) const;
  QString title() const; // the text of any contained title element
  QString label() const; // title or inkscape:label
private:
  QSharedDataPointer<XmlElementData> d;
};

#endif
