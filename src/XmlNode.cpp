// XmlNode.cpp

#include "XmlNode.h"
#include "XmlElement.h"

XmlNode::XmlNode(QXmlStreamReader &src): type_(Type::Invalid), element_(0) {
  if (src.isStartElement()) {
    type_ = Type::Element;
    element_ = QSharedPointer<XmlElement>(new XmlElement(src));
  } else if (src.isCharacters()) {
    type_ = Type::Text;
    text_ = src.text().toString();
  }
}

XmlNode::~XmlNode() {
}

void XmlNode::write(QXmlStreamWriter &writer) const {
  switch (type_) {
  case Type::Text:
    writer.writeCharacters(text_);
    break;
  case Type::Element:
    element_->write(writer);
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

XmlElement *XmlNode::element() const {
  return element_.data();
}
