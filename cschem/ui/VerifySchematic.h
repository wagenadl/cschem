// VerifySchematic.h

#ifndef VERIFYSCHEMATIC_H

#define VERIFYSCHEMATIC_H

class VerifySchematic {
public:
  VerifySchematic(class Scene *scene, class QWidget *parent);
  void run();
private:
  Scene *scene;
  QWidget *parent;
};

#endif
