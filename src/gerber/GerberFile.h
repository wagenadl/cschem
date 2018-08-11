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
  GerberFile(QDir dir, Gerber::Layer layer, QString uuid="");
  ~GerberFile();
  bool isValid() const;
  Gerber::Apertures &newApertures(Gerber::Apertures::Func);
  Gerber::Apertures const &apertures(Gerber::Apertures::Func) const;
  void writeApertures(Gerber::Apertures::Func);
  void writeApertures(Gerber::Apertures const &);
  Gerber::Font &ensureFont(Gerber::FontSpec);
  Gerber::Font const &font(Gerber::FontSpec) const;
public:
  static Gerber::Polarity filePolarity(Gerber::Layer);
  static QString fileFunction(Gerber::Layer);
private:
  void writeBoilerplate(class QDir const &dir,
			Gerber::Layer layer,
			QString uuid);
private:
  QFile f;
  int nextap;
  QMap<Gerber::Apertures::Func, Gerber::Apertures> aps;
  QMap<Gerber::FontSpec, Gerber::Font> fonts;
};

#endif
