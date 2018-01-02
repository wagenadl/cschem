// SceneConnection.h

#ifndef SCENECONNECTION_H

#define SCENECONNECTION_H

#include <QGraphicsItemGroup>
#include <QWeakPointer>
#include <QSharedPointer>

class SceneConnection: public QGraphicsItemGroup {
public:
  SceneConnection(class Scene *parent, class Connection const &);
  SceneConnection(SceneConnection const &) = delete;
  SceneConnection &operator=(SceneConnection const &) = delete;  
  ~SceneConnection();
  class WeakPtr {
  public:
    WeakPtr();
    SceneConnection *data() const;
    operator bool() const;
    void clear();
  private:
    SceneConnection *s;
    QWeakPointer<class SceneConnectionData> d;
  private:
    WeakPtr(SceneConnection *, QSharedPointer<SceneConnectionData> const &);
    friend class SceneConnection;
  };
  WeakPtr weakref();
public:
  class Scene *scene();
  int id() const;
  void rebuild();
  void temporaryTranslate(QPointF delta);
  void temporaryTranslateFrom(QPointF delta);
  void temporaryTranslateTo(QPointF delta);
  void setLineWidth(double frac = 1.0);
  int segmentAt(QPointF) const; // -1 if none
  void hover(int seg);
  void unhover();
protected:
  void mousePressEvent(QGraphicsSceneMouseEvent *) override;
  void mouseMoveEvent(QGraphicsSceneMouseEvent *) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *) override;
public:
  void paint(QPainter *, QStyleOptionGraphicsItem const *, QWidget *) override;
  QRectF boundingRect() const override;  
private:
  void setPath(class QPolygonF const &);
private:
  QSharedPointer<class SceneConnectionData> d;
};

#endif
