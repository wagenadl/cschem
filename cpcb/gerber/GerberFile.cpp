// GerberFile.cpp

#include "GerberFile.h"

using namespace Gerber;

GerberFile::GerberFile(QDir dir, Gerber::Layer layer, QString uuid,
		       bool useattr):
  f(dir.absoluteFilePath(dir.dirName() + "-" + layerInfix(layer) + "."
			 + layerSuffix(layer))),
  useattr(useattr) {
  if (!f.open(QFile::WriteOnly)) {
    qDebug() << "Could not create file" << f.fileName();
    return;
  }
  setDevice(&f);
  writeBoilerplate(dir, layer, uuid);
  nextap = 10;
}

GerberFile::~GerberFile() {
  // we do not write the M02 here, so that write errors are obvious
}

Gerber::Font &GerberFile::ensureFont(Gerber::FontSpec fs) {
  if (!aps.contains(Gerber::Apertures::Func::Material))
    newApertures(Gerber::Apertures::Func::Material);
  auto it = fonts.find(fs);
  if (it==fonts.end())
    return *fonts.insert(fs,
           Gerber::Font(&aps[Gerber::Apertures::Func::Material], fs));
  else
    return *it;
}

Gerber::Font const &GerberFile::font(Gerber::FontSpec fs) const {
  auto it = fonts.find(fs);
  if (it==fonts.end()) {
    qDebug() << "fontnotfound" << fs;
    for (Gerber::FontSpec f: fonts.keys())
      qDebug() << "  != " << f;
  }
  Q_ASSERT(it!=fonts.end());
  return *it;
}

Gerber::Polarity GerberFile::filePolarity(Gerber::Layer layer) {
  switch (layer) {
  case Gerber::Layer::BottomSolderMask:
  case Gerber::Layer::TopSolderMask:
    return Gerber::Polarity::Negative;
  default:
    return Gerber::Polarity::Positive;    
  }
}

QString GerberFile::fileFunction(Gerber::Layer layer) {
  switch (layer) {
  case Gerber::Layer::BoardOutline: return "Profile,NP";
  case Gerber::Layer::TopSilk: return "Legend,Top";
  case Gerber::Layer::ThroughHoles: return "Plated,1,2,PTH,Drill";
  case Gerber::Layer::NonplatedHoles: return "Nonplated,1,2,NPTH,Drill";
  case Gerber::Layer::TopCopper: return "Copper,L1,Top";
  case Gerber::Layer::BottomCopper: return "Copper,L2,Bot";
  case Gerber::Layer::TopSolderMask: return "Soldermask,Top";
  case Gerber::Layer::BottomSolderMask: return "Soldermask,Bot";
  case Gerber::Layer::TopPasteMask: return "Paste,Top";
  case Gerber::Layer::BottomPasteMask: return "Paste,Bot";
  }
  qDebug() << "unknown file function" << int(layer);
  return "XXX";
}

void GerberFile::writeBoilerplate(QDir const &dir,
				  Gerber::Layer layer,
				  QString uuid) {
  *this << "G04 " << dir.absolutePath() << " - " << layerInfix(layer) << " *\n";
  if (useattr)
    *this << "%TF.GenerationSoftware,cpcb,DanielWagenaar*%\n";
  *this << "%FSLAX35Y35*%\n";
  *this << "%MOMM*%\n";
  if (useattr) {
    *this << "%TF.FileFunction," << fileFunction(layer) << "*%\n";
    if (filePolarity(layer)==Polarity::Negative) 
      *this << "%TF.FilePolarity,Negative*%\n";
    else
      *this << "%TF.FilePolarity,Positive*%\n";
    *this << "%TF.Part,Single*%\n";
    if (!uuid.isEmpty())
      *this << "%TF.SameCoordinates," << uuid << "*%\n";
  }
}
    
bool GerberFile::isValid() const {
  qDebug() << "GerberFile::isValid?" << device();
  return device() != 0;
}

Gerber::Apertures &GerberFile::newApertures(Gerber::Apertures::Func func) {
  Q_ASSERT(!aps.contains(func));
  aps.insert(func, Gerber::Apertures(func, nextap, false));
  return aps[func];
}

Gerber::Apertures const &GerberFile::apertures(Gerber::Apertures::Func func)
  const {
  auto it = aps.find(func);
  Q_ASSERT(it!=aps.end());
  return *it;
}

void GerberFile::writeApertures(Gerber::Apertures::Func func) {
  if (aps.contains(func)) 
    writeApertures(aps[func]);
}

void GerberFile::writeApertures(Gerber::Apertures const &ap) {
  Q_ASSERT(ap.firstIndex() >= nextap);
  ap.write(*this);
  nextap = ap.nextIndex();
}
