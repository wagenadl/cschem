// FloatingPart.cpp

#include "FloatingPart.h"
#include "svg/Part.h"

class FloatingPartData {
public:
  FloatingPartData() {
  }
public:
  QPointF bbOrigin;
  QString name;
};

FloatingPart::FloatingPart(Part const &part, QPointF partpos):
  d(new FloatingPartData()) {
  d->bbOrigin = part.bbOrigin();
  d->name = part.name();
  setSharedRenderer(part.renderer().data());
  setPartPosition(partpos);
}
  
FloatingPart::~FloatingPart() {
  delete d;
}

void FloatingPart::setPartPosition(QPointF p) {
  setPos(p - d->bbOrigin);
}

QPointF FloatingPart::partPosition() const {
  return pos() + d->bbOrigin;
}

QString FloatingPart::name() const {
  return d->name;
}

