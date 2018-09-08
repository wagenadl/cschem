// XmlNode.cpp

#include "XmlNode.h"
#include "XmlElement.h"

class XmlNodeData: public QSharedData {
public:
  XmlNodeData(): type(XmlNode::Type::Invalid) { }
public:
  XmlNode::Type type;
  XmlElement element;
  QString text;
};

XmlNode::XmlNode() {
  d = new XmlNodeData;
}

XmlNode::XmlNode(XmlNode const &o) {
  d = o.d;
}

XmlNode &XmlNode::operator=(XmlNode const &o) {
  d = o.d;
  return *this;
}

XmlNode::XmlNode(QXmlStreamReader &src): XmlNode() {
  if (src.isStartElement()) {
    d->type = Type::Element;
    d->element = XmlElement(src);
  } else if (src.isCharacters()) {
    d->type = Type::Text;
    d->text = src.text().toString();
  }
}

XmlNode::~XmlNode() {
}

XmlNode::Type XmlNode::type() const {
  return d->type;
}
XmlElement const &XmlNode::element() const {
  return d->element;
}

XmlElement &XmlNode::element() {
  d.detach();
  return d->element;
}

QString XmlNode::text() const {
  return d->text;
}

void XmlNode::write(QXmlStreamWriter &writer) const {
  switch (d->type) {
  case Type::Text:
    writer.writeCharacters(d->text);
    break;
  case Type::Element:
    d->element.write(writer);
    break;
  default:
    break;
  }
}

QString XmlNode::toString() const {
  QString result;
  QXmlStreamWriter writer(&result);
  writer.setAutoFormatting(true);
  writer.setAutoFormattingIndent(2);
  writer.writeStartDocument("1.0", false);
  write(writer);
  writer.writeEndDocument();
  return result;
}
