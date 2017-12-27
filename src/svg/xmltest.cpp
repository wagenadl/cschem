// xmltest.cpp

#include "XmlElement.h"
#include "XmlNode.h"
#include <QDebug>

int main() {
  QFile file("../doc/libeg.svg");
  if (!file.open(QFile::ReadOnly)) {
    qDebug() << "file not found";
    return 1;
  }
  QXmlStreamReader sr(&file);
  while (!sr.atEnd()) {
    sr.readNext();
    if (sr.isStartElement()) {
      qDebug() << "working on" << sr.name();
      XmlNode node(sr);
      QString txt = node.toString();
      qDebug() << "looks like:" << txt.toUtf8().data();
    }
  }
}

      
