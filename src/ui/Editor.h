// Editor.h

#ifndef EDITOR_H

#define EDITOR_H

#include <QGraphicsView>

class Editor: public QGraphicsView {
public:
  Editor(class SymbolLibrary const *lib, class Schem *schem, QWidget *parent=0);
  ~Editor();
  void keyPressEvent(QKeyEvent *) override;
  class Scene *scene() const { return scene_; }
  class Schem const *schematic() const { return schem; }
  class SymbolLibrary const *library() const { return lib; }
private:
  SymbolLibrary const *lib;
  Schem *schem;
  class Scene *scene_;
};

#endif
