// Editor.cpp

#include "Editor.h"
#include "file/FileIO.h"
#include "file/Schem.h"
#include "Scene.h"

Editor::Editor(class SymbolLibrary const *lib, Schem *schem, QWidget *parent):
  QGraphicsView(parent), lib(lib), schem(schem) {
  setInteractive(true);
  scene_ = new Scene(lib);
  scene_->setCircuit(schem->circuit());
  setScene(scene_);
  scale(2, 2);
  setMouseTracking(true);
  setDragMode(RubberBandDrag);
  setHorizontalScrollbarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollbarPolicy(Qt::ScrollBarAlwaysOff);
  centerOn(scene_->sceneRect().center());
}

Editor::~Editor() {
  delete scene_;
}

void Editor::keyPressEvent(QKeyEvent *e) {
  bool take = false;
  if (e->modifiers() & Qt::ControlModifier) {
    take = true;
    switch (e->key()) {
    case Qt::Key_S:
      schem->circuit() = scene_->circuit();
      FileIO::saveSchematic("/tmp/eg.xml", *schem);
      break;
    case Qt::Key_Plus: case Qt::Key_Equal:
      scale(2, 2);
      break;
    case Qt::Key_Minus:
      scale(0.5, 0.5);
      break;
    default:
      take = false;
    }
  }
  if (take) {
    e->accept();
  } else {
    QGraphicsView::keyPressEvent(e);
  }
}
