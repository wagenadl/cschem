// PickNPlace.h

#ifndef PICKNPLACE_H

#define PICKNPLACE_H

#include "Group.h"
#include "Layout.h"
#include "circuit/Circuit.h"

class PNPLine {
public:
  PNPLine();
  PNPLine(Group const &g, Board const &board);
  void augment(Circuit const &circuit);
  bool isValid() const;
  QStringList toStringList() const;
  static QStringList header();
private:
  static QString dimToString(Dim x);
public:
  bool valid;
  QString ref;
  QString footprint;
  Point center;
  Point pin1;
  Rect bbox;
  int orient;
  QString comment;
};

class PickNPlace {
public:
  enum class Scope {
    SMTOnly,
    SMTAndThruHole,
  };
public:
  PickNPlace();
  PickNPlace(Layout const &layout, Scope scope=Scope::SMTOnly);
  void augment(Circuit const &circuit);
  QList<QStringList> toList() const;
  bool saveCSV(QString fn) const;
  QStringList placedRefs() const;
  QStringList unplacedRefs() const;
  QList<PNPLine> const &placed() const;
private:
  QList<PNPLine> lines;
  QStringList unrefs;
};

#endif
