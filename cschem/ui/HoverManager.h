// HoverManager.h

#ifndef HOVERMANAGER_H

#define HOVERMANAGER_H

#include <QPointF>
#include <QObject>

class HoverManager: public QObject {
  Q_OBJECT;
public:
  enum class Purpose {
    None, // to be used while dragging an element
    Connecting, // i.e., junctions and connections act as endpoints
    Moving, // i.e., junctions and connections act as movable
  };
public:
  HoverManager(class Scene *scene);
  HoverManager(HoverManager const &) = delete;
  HoverManager &operator=(HoverManager const &) = delete;
  ~HoverManager();
  void setPrimaryPurpose(Purpose);
  Purpose primaryPurpose() const;
  void unhover();
  void update();
  void update(QPointF);
  bool onElement() const; // that includes junctions
  bool onConnection() const;
  bool onPin() const;
  bool onFakePin() const;
  bool onNothing() const;
  int element() const; // -1 if none
  QString pin() const; // NOPIN if none
  int connection() const; // -1 if none
  int segment() const; // -1 if not on connection
  void formSelection(QSet<int> elts);
  QPoint tentativelyMoveSelection(QPoint delta, bool nomagnet=false);
  void newDrag(class Symbol const &);
  QPoint updateDrag(QPoint p);
  void doneDragging();
signals:
  void hoverChanged(QString); // message
private:
  friend class HoverManagerData;
  class HoverManagerData *d;
};

#endif
