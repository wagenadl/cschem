// PlaneEditor.h

#ifndef PLANEEDITOR_H

#define PLANEEDITOR_H

#include <QMouseEvent>
#include "data/Point.h"

class PlaneEditor {
public:
  PlaneEditor(class EData *);
  ~PlaneEditor();
  void resetMouseMargin();
  void render(class QPainter &);
  void mousePress(Point p, Qt::MouseButton b, Qt::KeyboardModifiers m);
  void mouseRelease(Point p, Qt::MouseButton b, Qt::KeyboardModifiers m);
  void mouseMove(Point p, Qt::MouseButton b, Qt::KeyboardModifiers m);
  void doubleClick(Point p, Qt::MouseButton b, Qt::KeyboardModifiers m);
  void deleteSelected();
private:
  class EData *ed;
  class PEData *d;
};

#endif
