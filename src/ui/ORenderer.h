// ORenderer.h

#ifndef ORENDERER_H

#define ORENDERER_H

#include "data/Object.h"

#include <QPainter>
#include <QTransform>

class ORenderer {
public:
  ORenderer(QPainter *painter, Point const &origin=Point());
  ~ORenderer();
  void setMoving(Point const &movingdelta);
  void setSelPoints(QSet<Point> const &);
  void setPurePoints(QSet<Point> const &);
  void setStuckPoints(QSet<Point> const &);
  void setLayer(Layer l);
  void pushOrigin(Point const &origin);
  void popOrigin();
  void drawObject(Object const &o, bool selected);
private:
  void drawText(Text const &t, bool selected);
  void drawTrace(Trace const &t, bool selected);
  void drawGroup(Group const &g, bool selected);
  void drawArc(Arc const &g, bool selected);
  void drawPad(Pad const &g, bool selected);
  void drawHole(Hole const &g, bool selected);
private:
  QPainter *p;
  Layer layer;
  QList<Point> originstack;
  Point origin;
  Point movingdelta;
  QSet<Point> stuckpts;
  QSet<Point> selpts;
  QSet<Point> purepts;
  bool toplevel;
};

#endif
