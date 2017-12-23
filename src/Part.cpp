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
      pinNames_.append(label.mid(4));
      QString x = elt->attributes().value("cx").toString();
      QString y = elt->attributes().value("cy").toString();
      pinPositions_.append(QPoint(x.toInt(), y.toInt()));
    }
  }
  for (auto e: elt->children()) 
    if (e->element())
      scanPins(e->element());
}

QPoint Part::pinPosition(QString pinname) const {
  for (int i=0; i<pinNames_.size(); i++)
    if (pinNames_[i]==pinname)
      return pinPositions_[i];
  return QPoint(0,0);
}

void Part::setBBox(QRectF b) {
  bbox_ = b;
}
