// FloatingPart.h

#ifndef FLOATINGPART_H

#define FLOATINGPART_H

#include <QGraphicsSvgItem>

class FloatingPart: public QGraphicsSvgItem {
public:
  FloatingPart(class Part const &part);
  ~FloatingPart();
  FloatingPart(FloatingPart const &) = delete;
  FloatingPart &operator=(FloatingPart const &) = delete;
  void setPartPosition(QPointF); // set position of first pin
  QPointF partPosition() const;
  QPointF shiftedCenter() const; // position of center relative to first pin
  QString name() const;
private:
  class FloatingPartData *d;
};

#endif
