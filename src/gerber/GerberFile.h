// GerberFile.h

#ifndef GERBERFILE_H

#define GERBERFILE_H

#include "Gerber.h"
#include <QTextStream>
#include <QDir>
#include <QFile>
#include "Apertures.h"

class GerberFile: public QTextStream {
public:
  GerberFile(QDir dir, Gerber::Layer layer);
  ~GerberFile();
  bool isValid() const;
  void writeBoilerplate(QString uuid,
			Gerber::Polarity polarity=Gerber::Polarity::Positive);
  Gerber::Apertures &newApertures(Gerber::Apertures::Func);
  Gerber::Apertures const &apertures(Gerber::Apertures::Func);
  void writeApertures(Gerber::Apertures::Func);
  void writeApertures(Gerber::Apertures const &);
private:
  QFile f;
  int nextap;
  QMap<Gerber::Apertures::Func, Gerber::Apertures> aps;
};

#endif
