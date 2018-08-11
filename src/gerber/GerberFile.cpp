// GerberFile.cpp

#include "GerberFile.h"

using namespace Gerber;

GerberFile::GerberFile(QDir dir, Gerber::Layer layer):
  f(dir.absoluteFilePath(dir.dirName() + "-" + layerInfix(layer) + ".gbr")),
  nextap(Gerber::Font::FirstSafeAperture),
  font_(newApertures(Gerber::Apertures::Func::NonConductor)) {
  if (!f.open(QFile::WriteOnly)) {
    qDebug() << "Could not create file" << f.fileName();
    return;
  }
  setDevice(&f);
  *this << "G04 " << dir.absolutePath() << " - " << layerInfix(layer) << " *\n";
  *this << "%TF.GenerationSoftware,cpcb,DanielWagenaar*%\n";
  *this << "%FSLAX35Y35*%\n";
  *this << "%MOMM*%\n";
}

GerberFile::~GerberFile() {
  // we do not write the M02 here, so that write errors are obvious
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

Gerber::Apertures const &GerberFile::apertures(Gerber::Apertures::Func func) {
  Q_ASSERT(aps.contains(func));
  return aps[func];
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

Gerber::Font &GerberFile::font() {
  return font_;
}
    
