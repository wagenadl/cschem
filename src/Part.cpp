// Part.cpp

#include "Part.h"

Part::Part(XmlElement const *elt): elt_(elt) {
  valid_ = false;
  name_ = elt->attributes().value("inkscape:label").toString();
  scanPins(elt);
}

void Part::scanPins(XmlElement const *elt) {
  if (elt->qualifiedName()=="circle") {
    QString label = elt->attributes().value("inkscape:label").toString();
    if (label.startsWith("pin")) {
      QString name = label.mid(4);
      QString x = elt->attributes().value("cx").toString();
      QString y = elt->attributes().value("cy").toString();
      pins_[name] = QPoint(x.toInt(), y.toInt());
    }
  }
  for (auto e: elt->children()) 
    if (e->element())
      scanPins(e->element());
}

QPoint Part::pinPosition(QString pinname) const {
  if (pins_.contains(pinname))
    return pins_[pinname] - bbox_.topLeft();
  else
    return QPoint();
}

void Part::setBBox(QRect b) {
  bbox_ = b;
}
