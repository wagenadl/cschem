// FPPicture.h

#ifndef FPPICTURE_H

#define FPPICTURE_H

#include <QPicture>

class FPPicture {
  /* This class interprets pcb/geda ".fp" files and can generate a QPicture
     from them. */
public:
  struct PinInfo {
    QString name;
    int number;
    QPoint position;
    int innerdiam;
    int outerdiam;
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
  QMap<QString, PinInfo> const &pins() const;
private:
  QSharedPointer<class FPPicData> d;
};

#endif
