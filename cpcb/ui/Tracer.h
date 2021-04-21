// Tracer.h

#ifndef TRACER_H

#define TRACER_H

#include "data/Layer.h"
#include "data/Point.h"

class Tracer {
public:
  Tracer(class EData *);
  ~Tracer();
  void start(Point const &); // start at exactly this point
  void pickup(Point const &);
  void click(Point const &);
  void move(Point const &);
  void confirm();
  void render(class QPainter &);
  void end();
  void setLayer(Layer const &);
  void setLinewidth(Dim const &);
  bool isTracing() const;
private:
  class TRData *d;
};

#endif
