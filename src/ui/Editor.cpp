// Editor.cpp

#include "Editor.h"
#include "file/FileIO.h"
#include "file/Schem.h"
#include "Scene.h"

Editor::Editor(class PartLibrary const *lib, Schem *schem, QWidget *parent):
  QGraphicsView(parent), lib(lib), schem(schem) {
  setInteractive(true);
  scene = new Scene(lib);
  scene->setCircuit(schem->circuit());
  setScene(scene);
  scale(2, 2);
}

Editor::~Editor() {
  delete scene;
}

void Editor::keyPressEvent(QKeyEvent *e) {
  bool take = false;
  if (e->modifiers() & Qt::ControlModifier) {
    take = true;
    switch (e->key()) {
    case Qt::Key_S:
      schem->circuit() = scene->circuit();
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
