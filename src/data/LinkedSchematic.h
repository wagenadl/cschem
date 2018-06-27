// LinkedSchematic.h

#ifndef LINKEDSCHEMATIC_H

#define LINKEDSCHEMATIC_H

#include <QString>

class LinkedSchematic {
public:
  LinkedSchematic();
  void link(QString fn);
  void unlink();
  ~LinkedSchematic();
  bool isValid() const;
private:
  LinkedSchematic(LinkedSchematic const &) = delete;
  LinkedSchematic &operator=(LinkedSchematic const &) = delete;
private:
  class LSData *d;
};

#endif
