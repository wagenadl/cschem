// Find.h

#ifndef FIND_H

#define FIND_H

class Find {
public:
  Find(class Editor *ed);
  bool run(); // returns true if found
private:
  Editor *ed;
};

#endif
