// Properties.h

#ifndef PROPERTIES_H

#define PROPERTIES_H

#include "data/Dim.h"
#include "data/Layer.h"

class Properties: QObject {
  /* Properties keeps the properties of the current object or whatever
     will be drawn next. */
  Q_OBJECT;
public:
  Properties(QObject *parent);
  virtual ~Properties();
  Dim grid() const;
  Dim lineWidth() const;
  Layer layer() const;
  int direction() const;
  bool isFlipped() const;
  Dim fontSize() const;
signals:
  void gridChanged(Dim);
  void lineWidthChanged(Dim);
  void layerChanged(Layer);
  void directionChanged(int);
  void flippedChanged(bool);
  void fontSizeChanged(Dim);
public slots:
  void setGrid(Dim const &);
  void setLineWidth(Dim const &);
  void setLayer(Layer);
  void setDirection(int);
  void setFlipped(bool);
  void setFontSize(Dim const &);
private:
  Dim grid_;
  Dim linewidth_;
  Dim layer_;
  int direction_;
  bool flipped_;
  Dim fontsize_;
  
};

#endif
