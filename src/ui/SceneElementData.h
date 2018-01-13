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
  }
  virtual ~SceneElementData();
public:
  void markHover();
public slots:
  void setNameText();
  void getNameText();
  void moveName(QPointF delta);
  void removeName();
  void setValueText();
  void getValueText();
  void moveValue(QPointF delta);
  void removeValue();
public:
  class Scene *scene;
  int id;
  QString sym;
  class QGraphicsSvgItem *element;
  class SceneAnnotation *name;
  class SceneAnnotation *value;
  QSharedPointer<class QSvgRenderer> renderer;
public:
  bool dragmoved;
  bool hover;
};

#endif
