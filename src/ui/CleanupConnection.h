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
  void cleanup(int con);
  /* This moves (or copies) junctions to remove overlapping segments.
   */
  QSet<int> affectedConnections(); // new, modified, or deleted
  QSet<int> affectedJunctions(); // new, modified, or deleted
  Circuit const &updatedCircuit();
private:
  class CCData *d;
};

#endif
