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
  void setStuckPoints(QMap<Layer, QSet<Point> > const &);
  void setLayer(Layer l);
  void pushOrigin(Point const &origin);
  void popOrigin();
  void drawObject(Object const &o, bool selected=false);
  void drawGroup(Group const &g, bool selected=false);
  void drawText(Text const &t, bool selected=false);
  void drawTrace(Trace const &t, bool selected=false);
  void drawArc(Arc const &g, bool selected=false);
  void drawPad(Pad const &g, bool selected=false);
  void drawHole(Hole const &g, bool selected=false);
public:
  static QByteArray objectToSvg(Object const &,
				  Dim margin=Dim(), Dim minSize=Dim());
  static void render(Group const &, QPainter *); // all layers
  static void render(Object const &, QPainter *); // all layers
private:
  QPainter *p;
  Layer layer;
  QList<Point> originstack;
  Point origin;
  Point movingdelta;
  QMap<Layer, QSet<Point> > stuckpts;
  QSet<Point> selpts;
  QSet<Point> purepts;
  bool toplevel;
};

#endif
