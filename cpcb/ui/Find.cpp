// Find.cpp

#include "Find.h"
#include "Editor.h"
#include "data/Group.h"
#include "data/Object.h"
#include <QInputDialog>
#include <QMessageBox>

Find::Find(Editor *ed): ed(ed) {
}

bool Find::run() {
  QString key = QInputDialog::getText(ed, "Find component or pin",
                                      "Ref. or pin name/number:",
                                      QLineEdit::Normal);
  if (key.isEmpty())
    return false;

  key = key.toLower();
  
  Group const &here(ed->currentGroup());
  for (int id: here.keys()) {
    Object const &obj(here.object(id));
    switch (obj.type()) {
    case Object::Type::Group:
      if (obj.asGroup().ref.toLower()==key) {
        ed->select(id);
        int id_reftext = obj.asGroup().refTextId();
        if (id_reftext>0)
          ed->select(id_reftext, true);
        return true;
      }
      break;
    case Object::Type::Hole:
      if (obj.asHole().ref.toLower()==key) {
        ed->select(id);
        return true;
      }
      break;
    case Object::Type::Pad:
      if (obj.asPad().ref.toLower()==key) {
        ed->select(id);
        return true;
      }
      break;
    default:
      break;
    }
  }
  QMessageBox::information(ed, "Find", "Not found");
  return false;
}
