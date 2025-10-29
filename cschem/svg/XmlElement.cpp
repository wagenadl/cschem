// XmlElement.cpp

#include "XmlElement.h"
#include <QDebug>

class XmlElementData: public QSharedData {
public:
  XmlElementData(): valid(false) {}
public:
  bool valid;
  QString name;
  QXmlStreamAttributes attributes;
  QXmlStreamNamespaceDeclarations namespaceDeclarations;
  QList<XmlNode> children;
};

bool XmlElement::removeAttribute(QString name) {
  for (int k=0; k<d->attributes.size(); k++) {
    if (d->attributes[k].name() == name) {
      d->attributes.remove(k);
      return true;
    }
  }
  return false;
}

XmlElement::XmlElement() {
  d = new XmlElementData();
}

XmlElement::XmlElement(XmlElement const &o) {
  d = o.d;
}

XmlElement &XmlElement::operator=(XmlElement const &o) {
  d = o.d;
  return *this;
}

XmlElement::~XmlElement() {
}

XmlElement::XmlElement(QXmlStreamReader &src): XmlElement() {
  if (!src.isStartElement())
    return;
  d->name = src.name().toString();
  d->attributes = src.attributes();
  d->namespaceDeclarations = src.namespaceDeclarations();
  while (!src.atEnd()) {
    src.readNext();
    if (src.isEndElement()) {
      if (src.name() == d->name) {
        d->valid = true;
        return;
      } else {
        qDebug() << "Unexpected closing tag at line" << src.lineNumber();
        return;
      }
    } else if (src.isEndDocument()) {
      qDebug() << "Unexpected end of document at line" << src.lineNumber();
    } else {
      d->children.append(XmlNode(src));
    }
  }
}

bool XmlElement::isValid() const {
  return d->valid;
}

QList<XmlNode> const &XmlElement::children() const {
  return d->children;
}

QString XmlElement::name() const {
  return d->name;
}

QXmlStreamAttributes XmlElement::attributes() const {
  return d->attributes;
}

QXmlStreamNamespaceDeclarations XmlElement::namespaceDeclarations() const {
  return d->namespaceDeclarations;
}


void XmlElement::write(QXmlStreamWriter &dst) const {
  writeStartElement(dst);
  writeChildren(dst);
  writeEndElement(dst);
}
  
void XmlElement::writeStartElement(QXmlStreamWriter &dst) const {
  dst.writeStartElement(d->name);
  dst.writeAttributes(d->attributes);
  for (auto nsd: d->namespaceDeclarations) {
    if (nsd.prefix().isEmpty()) {
      dst.writeDefaultNamespace(nsd.namespaceUri().toString());
    } else {
      dst.writeNamespace(nsd.namespaceUri().toString(), nsd.prefix().toString());
    }
  }
}

void XmlElement::writeChildren(QXmlStreamWriter &dst) const {
  for (auto &c: d->children)
    c.write(dst);
}

void XmlElement::writeEndElement(QXmlStreamWriter &dst) const {
  dst.writeEndElement();
}

QString XmlElement::title() const {
  for (auto const &c: children()) {
    if (c.type()==XmlNode::Type::Element) {
      auto const &elt = c.element();
      if (elt.name() == "title") {
        for (auto const &c: elt.children())
          if (c.type()==XmlNode::Type::Text)
            return c.text();
      } else {
        return ""; // title must be first element
      }
    }
  }
  return "";
}

QString XmlElement::label() const {
  QString txt = title();
  if (txt.isEmpty())
    txt = attributes().value("inkscape:label").toString();
  return txt;
}

void XmlElement::setTitle(QString t) {
  for (XmlNode &c: d->children) {
    if (c.type()==XmlNode::Type::Element) {
      XmlElement &elt = c.element();
      if (elt.name() == "title") {
        for (XmlNode &c: elt.d->children) {
          if (c.type()==XmlNode::Type::Text) {
            c.setText(t);
            return;
          }
        }
      } else {
        break; // title must be first element
      }
    }
  }
  XmlElement ttlelt;
  ttlelt.d->name = "title";
  ttlelt.d->valid = true;
  ttlelt.d->children << XmlNode::textNode(t);
  XmlNode ttl(XmlNode::elementNode(ttlelt));
  d->children.push_front(ttl);
}
