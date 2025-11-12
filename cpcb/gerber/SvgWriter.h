// SvgWriter.h

#ifndef SVGWRITER_H

#define SVGWRITER_H

#include <QTextStream>
#include <QFile>
#include "data/Dim.h"
#include "data/Rect.h"
#include "data/Trace.h"
#include <QColor>

class SvgWriter {
public:
  SvgWriter(QFile *file, Dim width, Dim height);
  ~SvgWriter();
  bool isValid() const;
  SvgWriter &operator<<(QString s);
  void drawTrace(Point const &p1, Point const &p2, Dim const &width,
                 QColor const &color);
  void drawRect(Rect const &rect, QColor const &color,
                Dim width=Dim::fromInch(0.2/96));
  void fillRect(Rect const &rect, QColor color);
  void fillRing(Point const &center, Dim inner, Dim outer, // diams
                QColor const &color);
  void drawText(QString text, Point const &anchor, QColor const &color,
                Dim fontsize=Dim::fromInch(11.0/72), int angle_degcw=0);
public:
  static QString color(QColor const &x);
  static QString coord(Dim const &x);
  static QString prop(QString name, Dim const &x);
private:
  void writeHeader(Dim width, Dim height);
  void writeFooter();
private:
  QTextStream stream;
};

#endif
