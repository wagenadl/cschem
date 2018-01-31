// SceneElementData.h

#ifndef SCENEELEMENTDATA_H

#define SCENEELEMENTDATA_H
#include <QObject>
#include <QPointF>
#include <QSharedPointer>

class SceneElementData: public QObject {
  Q_OBJECT;
public:
  SceneElementData() {
    scene = 0;
    id = 0;
    element = 0;
    name = 0;
    value = 0;
    dragmoved = false;
    hover = false;
    nhover = vhover = false;
  }
  virtual ~SceneElementData();
public:
  void markHover();
public slots:
  void nameTextToWidget();
  void nameTextToCircuit();
  void moveName(QPointF delta);
  void removeName();
  void valueTextToWidget();
  void valueTextToCircuit();
  void moveValue(QPointF delta);
  void removeValue();
  void nameHovering(bool);
  void valueHovering(bool);
public:
  class Scene *scene;
  int id;
  QString sym;
  class SvgItem *element;
  class SceneAnnotation *name;
  class SceneAnnotation *value;
  QPoint delta0;
public:
  bool dragmoved;
  bool hover;
  bool nhover, vhover;
};

#endif
