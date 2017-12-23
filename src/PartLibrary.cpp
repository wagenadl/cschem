// PartLibrary.cpp

#include "PartLibrary.h"
#include <QDebug>
#include <QSvgRenderer>
#include <math.h>

PartLibrary::PartLibrary(QString fn) {
  svg_ = 0;
  
  QFile file(fn);
  if (!file.open(QFile::ReadOnly)) {
    qDebug() << "Failed to open part library";
    return;
  }

  QXmlStreamReader sr(&file);
  while (!sr.atEnd()) {
    sr.readNext();
    if (sr.isStartElement() && sr.qualifiedName()=="svg") {
      svg_ = new XmlElement(sr);
      break;
    }
  }

  if (!svg_) {
    qDebug() << "Failed to parse svg";
    return;
  }

  scanParts(svg_);

  getBBoxes(fn);

  for (auto p: partslist_) {
    qDebug() << "Relative pins for" << p->name() << p->bbox().size();
    for (auto n: p->pins().keys())
      qDebug() << "  " << n << p->pinPosition(n);
  }
}

PartLibrary::~PartLibrary() {
  for (auto p: partslist_)
    delete p;
}

void PartLibrary::scanParts(XmlElement const *src) {
  if (src->qualifiedName()=="g") {
    QString label = src->attributes().value("inkscape:label").toString();
    if (label.startsWith("part:") || label.startsWith("port:")) {
      Part *part = new Part(src);
      partslist_.append(part);
      parts_[part->name()] = part;
      qDebug() << "Got part" << part->name() << "with pins" << part->pins();
    }
  }
  for (auto c: src->children())
    if (c->element())
      scanParts(c->element());
}

void PartLibrary::getBBoxes(QString fn) {
  QSvgRenderer svg(fn);
  for (auto p: partslist_) {
    QString id = p->element()->attributes().value("id").toString();
    p->setBBox(svg.boundsOnElement(id).toAlignedRect().adjusted(-2,-2,2,2));
    qDebug() << "bbox on " << p->name() << id << p->bbox();
  }
}

QString PartLibrary::partSvg(QString name) {
  if (!parts_.contains(name))
    return "";

  Part const *p = parts_[name];
  QRectF bb = p->bbox();
  XmlElement const *elt = p->element();

  QString res;
  {
    QXmlStreamWriter sr(&res);
    sr.writeStartDocument("1.0", false);
    sr.writeStartElement("svg");
    for (auto nsd: svg_->namespaceDeclarations()) {
      if (nsd.prefix()=="")
        sr.writeDefaultNamespace(nsd.namespaceUri().toString());
      else
        sr.writeNamespace(nsd.namespaceUri().toString(),
                          nsd.prefix().toString());
    }
    sr.writeAttribute("width", QString("%1").arg(bb.width()));
    sr.writeAttribute("height", QString("%1").arg(bb.height()));
    sr.writeAttribute("viewBox", QString("0 0 %1 %2")
                      .arg(bb.width()).arg(bb.height()));

    sr.writeStartElement("g");
    sr.writeAttribute("transform",
                      QString("translate(%1,%2)").
                      arg(-bb.left()).arg(-bb.top()));

    /* This should be improved to suppress the pins */
    elt->writeStartElement(sr);
    for (auto c: elt->children()) {
      if (c->element()
          && c->element()->attributes().value("inkscape:label").startsWith("pin")) {
        // skip
      } else {
        c->write(sr);
      }
    }
    elt->writeEndElement(sr);

    sr.writeEndElement(); // g [transform]
    sr.writeEndElement(); // svg
    sr.writeEndDocument();
  }
  
  return res;
}
