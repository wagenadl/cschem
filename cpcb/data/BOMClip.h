// BOMClip.h

#ifndef BOMCLIP_H

#define BOMCLIP_H

#include <QMap>
#include <QPoint>
#include <QMimeData>

class BOMClip {
  // This is the global clipboard for selections from the BOM
public:
  struct Cell {
    int dx, dy;
    QString text;
    Cell(int dx=0, int dy=0, QString text=""): dx(dx), dy(dy), text(text) {}
  };
  typedef QList<Cell> CellList;
public:
  static BOMClip &instance();
  static constexpr char const *dndformat = "application/x-dnd-cpcb-bomcells";
  static QMimeData *createMimeData(CellList const &);
  static CellList parseMimeData(QMimeData const *);
public:
  bool isValid() const;
  void store(CellList const &);
  CellList retrieve() const;
private:
  BOMClip();
};

#endif
