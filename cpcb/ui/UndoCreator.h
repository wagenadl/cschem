// UndoCreator.h

#ifndef UNDOCREATOR_H

#define UNDOCREATOR_H

#include "EData.h"
#include "Editor.h"

class UndoCreator {
public:
  UndoCreator(EData *d, bool imm=false): d(d), realized(false) {
    if (imm)
      realize();
  }
  ~UndoCreator() {
    if (realized) {
      d->ed->changedFromSaved(d->stepsfromsaved != 0);
      d->ed->undoAvailable(true);
      d->ed->redoAvailable(false);
      d->ed->update();
    }
  }
  void realize() {
    if (!realized) {
      d->createUndoPoint();
      realized = true;
    }
  }
private:
  EData *d;
  bool realized;
};    

#endif
