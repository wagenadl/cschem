// SvgWriter.h

#ifndef SVGWRITER_H

#define SVGWRITER_H

#include <QTextStream>
#include <QFile>
#include "data/Dim.h"
#include "data/Rect.h"
#include "data/Trace.h"
#include "data/Hole.h"
#include "data/NPHole.h"
#include "data/Arc.h"
#include "data/FilledPlane.h"
#include "data/Pad.h"
#include <QColor>

class SvgWriter {
public:
  SvgWriter(QFile *file, Dim width, Dim height);
  ~SvgWriter();
  bool isValid() const;
  SvgWriter &operator<<(QString s);
  // low-level drawing operations
  void drawSegment(Point const &p1, Point const &p2, Dim const &width,
                 QColor const &color);
  void drawRect(Rect const &rect, QColor const &color,
                Dim width=Dim::fromInch(0.2/96));
  void fillRect(Rect const &rect, QColor const &color);
  void drawCircle(Point const &center, Dim const &diam, QColor const &color,
                Dim width=Dim::fromInch(0.2/96));
  void fillCircle(Point const &center, Dim const &diam, QColor const &color);
  void drawText(QString text, Point const &anchor, QColor const &color,
                Dim fontsize=Dim::fromInch(11.0/72), double angle_degcw=0);
  void startGroup(Point const &translate=Point(), double angle_degcw=0);
  void endGroup();
  // high-level drawing operations
  void renderPad(Pad const &);
  void renderHole(Hole const &);
  void renderTrace(Trace const &);
  void renderNPHole(NPHole const &);
  void renderArc(Arc const &);
  void renderPlane(FilledPlane const &);
public:
 //static QString color(QColor const &x);
 //static QString coord(Dim const &x);
 //static QString prop(QString name, Dim const &x);
private:
  void writeHeader(Dim width, Dim height);
  void writeFooter();
private:
  QTextStream stream;
};

#endif
