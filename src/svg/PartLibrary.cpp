// PartLibrary.cpp

#include "PartLibrary.h"
#include <QDebug>
#include <QSvgRenderer>
#include <math.h>

PartLibrary::PartLibrary(QString fn) {
  QFile file(fn);
  if (!file.open(QFile::ReadOnly)) {
    qDebug() << "Failed to open part library";
    return;
  }

  QXmlStreamReader sr(&file);
  while (!sr.atEnd()) {
    sr.readNext();
    if (sr.isStartElement() && sr.qualifiedName()=="svg") {
      svg_ = XmlElement(sr);
      break;
    }
  }

  if (!svg_.isValid()) {
    qDebug() << "Failed to parse svg";
    return;
  }

  scanParts(svg_);

  getBBoxes(fn);

  for (auto p: partslist_) {
    qDebug() << "Relative pins for" << p.name() << p.bbox().size();
    for (auto n: p.pinNames())
      qDebug() << "  " << n << p.pinPosition(n);
  }
}

PartLibrary::~PartLibrary() {
  for (auto r: renderers_)
    delete r;
}

void PartLibrary::scanParts(XmlElement const &src) {
  if (src.qualifiedName()=="g") {
    QString label = src.attributes().value("inkscape:label").toString();
    if (label.startsWith("part:") || label.startsWith("port:")
        || label=="junction") {
      partslist_.append(Part(src));
      Part const *part = &partslist_.last();
      parts_[part->name()] = part;
      qDebug() << "Got part" << part->name() << "with pins" << part->pinNames();
    }
  }
  for (auto &c: src.children())
    if (c.type()==XmlNode::Type::Element)
      scanParts(c.element());
}

void PartLibrary::getBBoxes(QString fn) {
  QSvgRenderer svg(fn);
  for (auto &p: partslist_) {
    QString id = p.contentsSvgId();
    if (id=="")
      id = p.element().attributes().value("id").toString();
    p.setBBox(svg.boundsOnElement(id).toAlignedRect().adjusted(-2, -2, 2, 2));
    qDebug() << "bbox on " << p.name() << id << p.bbox();
    for (QString pin: p.pinNames()) {
      QString id = p.pinSvgId(pin);
      if (id!="") {
        auto c = svg.boundsOnElement(id).center();
        p.setAbsPinPosition(pin, c);
      }
    }
  }
}

QStringList PartLibrary::partNames() const {
  return parts_.keys();
}

Part PartLibrary::part(QString name) const {
  if (parts_.contains(name))
    return *parts_[name];
  else
    return Part();
}

QByteArray PartLibrary::partSvg(QString name) const {
  if (!parts_.contains(name))
    return "";

  Part const *p = parts_[name];
  QRectF bb = p->bbox();
  XmlElement const &elt = p->element();

  QByteArray res;
  {
    QXmlStreamWriter sr(&res);
    sr.writeStartDocument("1.0", false);
    sr.writeStartElement("svg");
    for (auto nsd: svg_.namespaceDeclarations()) {
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
    elt.writeStartElement(sr);
    for (auto &c: elt.children()) {
      if (c.type()==XmlNode::Type::Element
          && c.element().attributes().value("inkscape:label").startsWith("pin")) {
        // skip
      } else {
        c.write(sr);
      }
    }
    elt.writeEndElement(sr);

    sr.writeEndElement(); // g [transform]
    sr.writeEndElement(); // svg
    sr.writeEndDocument();
  }
  
  return res;
}

QSvgRenderer *PartLibrary::renderer(QString name) const {
  if (renderers_.contains(name))
    return renderers_[name];

  if (!parts_.contains(name))
    return 0;

  QSvgRenderer *r = new QSvgRenderer(partSvg(name));
  renderers_[name ] = r;
  return r;
}

int PartLibrary::scale() const {
  return 7; // infer this from part drawings?
}

QPoint PartLibrary::downscale(QPointF p) const {
  return (p/scale()).toPoint();
}

QPointF PartLibrary::upscale(QPoint p) const {
  return QPointF(p*scale());
}

QPointF PartLibrary::nearestGrid(QPointF p) const {
  return upscale(downscale(p));
}

QRect PartLibrary::downscale(QRectF r) const {
  return QRect(downscale(r.topLeft()), downscale(r.bottomRight()));
}

QRectF PartLibrary::upscale(QRect r) const {
  return QRectF(upscale(r.topLeft()), upscale(r.bottomRight()));
}

double PartLibrary::lineWidth() const {
  return 2.5;
}
