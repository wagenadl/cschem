// Tracer.h

#ifndef TRACER_H

#define TRACER_H

#include "data/Layer.h"

class Tracer {
public:
  Tracer(class EData *);
  ~Tracer();
  void start(class Point const &); // start at exactly this point
  void pickup(class Point const &);
  void click(class Point const &);
  void move(class Point const &);
  void confirm();
  void render(class QPainter &);
  void end();
  void setLayer(Layer const &);
  void setLinewidth(class Dim const &);
  bool isTracing() const;
private:
  class TRData *d;
};

#endif
