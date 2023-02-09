// UndoCreator.h

#ifndef UNDOCREATOR_H

#define UNDOCREATOR_H

#include "EData.h"
#include "Editor.h"

class UndoCreator {
public:
  UndoCreator(EData *d, bool imm=false): d(d), realized(false) {
    d->undocreatorstackdepth++;
    if (imm)
      realize();
  }
  UndoCreator(Editor *ed, bool imm=false): d(ed->d), realized(false) {
    d->undocreatorstackdepth++;
    if (imm)
      realize();
  }
  ~UndoCreator() {
    if (realized && d->undocreatorstackdepth<=1) {
      d->ed->changedFromSaved(d->stepsfromsaved != 0);
      d->ed->undoAvailable(true);
      d->ed->redoAvailable(false);
      d->ed->update();
    }
    d->undocreatorstackdepth--;
  }
  void realize() {
    if (!realized && d->undocreatorstackdepth<=1) {
      d->createUndoPoint();
      realized = true;
    }
  }
private:
  EData *d;
  bool realized;
};    

#endif
