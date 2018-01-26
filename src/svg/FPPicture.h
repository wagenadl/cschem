// FPPicture.h

#ifndef FPPICTURE_H

#define FPPICTURE_H

#include <QPicture>

class FPPicture {
  /* This class interprets pcb/geda ".fp" files and can generate a QPicture
     from them. */
public:
  struct PinInfo {
    PinInfo() {
      number = 0;
      flags = 0;
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
  FPPicture(QString fn);
  ~FPPicture();
  FPPicture(FPPicture const &);
  FPPicture &operator=(FPPicture const &);
  QPicture picture() const;
  QString name() const; /* e.g., "ACY100P". The docs at
		   http://pcb.geda-project.org/pcb-cvs/pcb.html#Element-syntax
		   call this "value". */
  QString description() const; // e.g., "Axial polar component..."
  QMap<int, PinInfo> const &pins() const; // organized by number
private:
  QSharedPointer<class FPPicData> d;
};

#endif
