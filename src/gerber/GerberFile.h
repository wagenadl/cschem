// GerberFile.h

#ifndef GERBERFILE_H

#define GERBERFILE_H

#include "Gerber.h"
#include "Font.h"
#include "Apertures.h"

#include <QTextStream>
#include <QDir>
#include <QFile>

class GerberFile: public QTextStream {
public:
  GerberFile(QDir dir, Gerber::Layer layer);
  ~GerberFile();
  bool isValid() const;
  void writeBoilerplate(QString uuid,
			Gerber::Polarity polarity=Gerber::Polarity::Positive);
  Gerber::Apertures &newApertures(Gerber::Apertures::Func);
  Gerber::Apertures const &apertures(Gerber::Apertures::Func) const;
  void writeApertures(Gerber::Apertures::Func);
  void writeApertures(Gerber::Apertures const &);
  Gerber::Font &ensureFont(Gerber::FontSpec);
  Gerber::Font const &font(Gerber::FontSpec) const;
private:
  QFile f;
  int nextap;
  QMap<Gerber::Apertures::Func, Gerber::Apertures> aps;
  QMap<Gerber::FontSpec, Gerber::Font> fonts;
};

#endif
