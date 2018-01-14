// FloatingPart.cpp

#include "FloatingPart.h"
#include "svg/Part.h"

class FloatingPartData {
public:
  FloatingPartData() {
  }
public:
  QPointF bbOrigin;
  QPointF sCM;
  QString name;
};

FloatingPart::FloatingPart(Part const &part):
  d(new FloatingPartData()) {
  d->bbOrigin = part.bbOrigin();
  d->sCM = part.shiftedBBox().center();
  d->name = part.name();
  setSharedRenderer(part.renderer().data());
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

QPointF FloatingPart::shiftedCenter() const {
  return d->sCM;
}

QString FloatingPart::name() const {
  return d->name;
}

