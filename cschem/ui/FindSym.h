// FindSym.h

#ifndef FINDSYM_H

#define FINDSYM_H

class FindSym {
public:
  FindSym(class Scene *scene, class QWidget *parent);
  bool run(); // returns true if found
private:
  Scene *scene;
  class QWidget *parent;
};

#endif
