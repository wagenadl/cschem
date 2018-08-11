// GerberFile.cpp

#include "GerberFile.h"

using namespace Gerber;

GerberFile::GerberFile(QDir dir, Gerber::Layer layer):
  f(dir.absoluteFilePath(dir.dirName() + "-" + layerInfix(layer) + ".gbr")) {
  if (!f.open(QFile::WriteOnly)) {
    qDebug() << "Could not create file" << f.fileName();
    return;
  }
  setDevice(&f);
  *this << "G04 " << dir.absolutePath() << " - " << layerInfix(layer) << " *\n";
  *this << "%TF.GenerationSoftware,cpcb,DanielWagenaar*%\n";
  *this << "%FSLAX35Y35*%\n";
  *this << "%MOMM*%\n";
  nextap = 10;
}

GerberFile::~GerberFile() {
  // we do not write the M02 here, so that write errors are obvious
}

Gerber::Font &GerberFile::ensureFont(Gerber::FontSpec fs) {
  if (!aps.contains(Gerber::Apertures::Func::NonConductor))
    newApertures(Gerber::Apertures::Func::NonConductor);
  auto it = fonts.find(fs);
  if (it==fonts.end())
    return *fonts.insert(fs,
           Gerber::Font(&aps[Gerber::Apertures::Func::NonConductor], fs));
  else
    return *it;
}

Gerber::Font const &GerberFile::font(Gerber::FontSpec fs) const {
  auto it = fonts.find(fs);
  Q_ASSERT(it!=fonts.end());
  return *it;
}

void GerberFile::writeBoilerplate(QString uuid, Polarity polarity) {
  if (polarity==Polarity::Negative) 
    *this << "%TF.FilePolarity,Negative*%\n";
  else
    *this << "%TF.FilePolarity,Positive*%\n";
  *this << "%TF.Part,Single*%\n";
  *this << "%TF.SameCoordinates," << uuid << "*%\n";
}
    
bool GerberFile::isValid() const {
  qDebug() << "GerberFile::isValid?" << device();
  return device() != 0;
}

Gerber::Apertures &GerberFile::newApertures(Gerber::Apertures::Func func) {
  Q_ASSERT(!aps.contains(func));
  aps.insert(func, Gerber::Apertures(func, nextap));
  return aps[func];
}

Gerber::Apertures const &GerberFile::apertures(Gerber::Apertures::Func func)
  const {
  auto it = aps.find(func);
  Q_ASSERT(it!=aps.end());
  return *it;
}

void GerberFile::writeApertures(Gerber::Apertures::Func func) {
  Q_ASSERT(aps.contains(func));
  writeApertures(aps[func]);
}

void GerberFile::writeApertures(Gerber::Apertures const &ap) {
  Q_ASSERT(ap.firstIndex() >= nextap);
  ap.write(*this);
  nextap = ap.nextIndex();
}
