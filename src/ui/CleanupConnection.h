// CleanupConnection.h

#ifndef CLEANUPCONNECTION_H

#define CLEANUPCONNECTION_H

#include <QSet>

class CleanupConnection {
public:
  CleanupConnection(class Scene *scene);
  CleanupConnection(CleanupConnection const &) = delete;
  CleanupConnection &operator=(CleanupConnection const &) = delete;
  ~CleanupConnection();
public:
  QSet<int> affectedConnections(); // new, modified, or deleted
  QSet<int> affectedElements(); // new, modified, or deleted
  Circuit const &updatedCircuit();
public:
  void deleteConnection(int con);
  void deleteElement(int elt);
  void removeImmediateRedundancy(int con);
private:
  class CCData *d;
};

#endif
