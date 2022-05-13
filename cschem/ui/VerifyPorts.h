// VerifyPorts.h

#ifndef VERIFYPORTS_H

#define VERIFYPORTS_H

class VerifyPorts {
public:
  VerifyPorts(class Scene *scene, class QWidget *parent);
  void run();
private:
  Scene *scene;
  QWidget *parent;
};

#endif
