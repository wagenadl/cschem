// XmlElement.cpp

#include "XmlElement.h"
#include <QDebug>

XmlElement::XmlElement(QXmlStreamReader &src):
  valid_(false) {
  if (!src.isStartElement())
    return;
  qualifiedName_ = src.qualifiedName().toString();
  attributes_ = src.attributes();
  namespaceDeclarations_ = src.namespaceDeclarations();
  while (!src.atEnd()) {
    src.readNext();
    if (src.isEndElement()) {
      if (src.qualifiedName() == qualifiedName_) {
        valid_ = true;
        return;
      } else {
        qDebug() << "Unexpected closing tag at line" << src.lineNumber();
        return;
      }
    } else if (src.isEndDocument()) {
      qDebug() << "Unexpected end of document at line" << src.lineNumber();
    } else {
      children_.append(XmlNode(src));
    }
  }
}
     
XmlElement::~XmlElement() {
}

void XmlElement::write(QXmlStreamWriter &dst) const {
  writeStartElement(dst);
  writeChildren(dst);
  writeEndElement(dst);
}
  
void XmlElement::writeStartElement(QXmlStreamWriter &dst) const {
  dst.writeStartElement(qualifiedName_);
  dst.writeAttributes(attributes_);
  for (auto nsd: namespaceDeclarations_) {
    if (nsd.prefix()=="") {
      qDebug() << "writing default namespace" << nsd.namespaceUri();
      dst.writeDefaultNamespace(nsd.namespaceUri().toString());
    } else {
      qDebug() << "writing namespace" << nsd.prefix() << nsd.namespaceUri();
      dst.writeNamespace(nsd.namespaceUri().toString(), nsd.prefix().toString());
    }
  }
}

void XmlElement::writeChildren(QXmlStreamWriter &dst) const {
  for (auto &c: children_)
    c.write(dst);
}

void XmlElement::writeEndElement(QXmlStreamWriter &dst) const {
  dst.writeEndElement();
}
