// FPPicture.h

#ifndef FPPICTURE_H

#define FPPICTURE_H

#include <QPicture>

class PackageDrawing {
  /* This class interprets pcb/geda ".fp" files and can generate a QPicture
     from them. */
public:
  struct PinInfo {
    PinInfo() {
      number = 0;
      isSquare = false;
      drillDiameter =0;
      padDiameter = 0;
      maskDiameter = 0;
      clearanceDiameter = 0;
    }
    QString name;
    int number;
    bool isSquare;
    QPoint position;
    int drillDiameter;
    int padDiameter;
    int maskDiameter;
    int clearanceDiameter;
  };
public:
  PackageDrawing();
  PackageDrawing(QString fn);
  ~PackageDrawing();
  PackageDrawing(PackageDrawing const &);
  PackageDrawing &operator=(PackageDrawing const &);
  QPicture picture() const;
  QString name() const; /* e.g., "ACY100P". The docs at
		   http://pcb.geda-project.org/pcb-cvs/pcb.html#Element-syntax
		   call this "value". */
  QString description() const; // e.g., "Axial polar component..."
  QMap<int, PinInfo> const &pins() const; // organized by number
  bool isValid() const;
private:
  QSharedPointer<class FPPicData> d;
};

#endif
