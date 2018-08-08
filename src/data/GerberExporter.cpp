// GerberExporter.cpp

#include "GerberExporter.h"
#include "Layout.h"
#include <QTextStream>

class GEData {
public:
  static constexpr qint64 PerUM = 100;
  static constexpr qint64 FromDim = PerUM / Dim::PerUM;
  static QString real(Dim const &d) { return QString::number(d.toMM()); }
  static QString coord(Dim const &d) { return QString::number(d.raw()*FromDim); }
  static Dim fontLineWidth(Text const &t) {
    static SimpleFont const &sf(SimpleFont::instance());
    return t.fontsize * sf.baseLinewidth() / sf.baseSize();
  }
public:
public:
  GEData(QTextStream &output): output(output) {
    apidx = 10;
  }
  void collectApertures(Group const &);
  void ensureAperture(CircKey);
  void ensureAperture(RectKey);
  void ensureAperture(HoleKey);
  void ensureAperture(SqHoleKey);
  void ensureCharacter(char c);
  void selectAperture(CircKey);
  void selectAperture(RectKey);
  void selectAperture(HoleKey);
  void selectAperture(SqHoleKey);
public:
  QTextStream &output;
  int apidx;
  QMap<CircKey, int> apCirc;
  QMap<RectKey, int> apRect;
  QMap<HoleKey, int> apHole;
  QMap<SqHoleKey, int> apSqHole;
  QMap<char, int> amChar;
};

void GEData::selectAperture(CircKey k) {
  Q_ASSERT(apCirc.contains(k));
  output << "%D" << apCirc[k] << "*%\n";
}

void GEData::selectAperture(HoleKey k) {
  Q_ASSERT(apHole.contains(k));
  output << "%D" << apHole[k] << "*%\n";
}

void GEData::selectAperture(RectKey k) {
  Q_ASSERT(apRect.contains(k));
  output << "%D" << apRect[k] << "*%\n";
}

void GEData::selectAperture(SqHoleKey k) {
  Q_ASSERT(apSqHole.contains(k));
  output << "%D" << apSqHole[k] << "*%\n";
}

void GEData::ensureAperture(CircKey k) {
  if (apCirc.contains(k))
    return;
  apCirc[k] = ++apidx;
  output << "%ADD" << apidx << "C," << real(k.diam) << "*%\n";
}

void GEData::ensureAperture(HoleKey k) {
  if (apHole.contains(k))
    return;
  apHole[k] = ++apidx;
  output << "%ADD" << apidx << "C," << real(k.od) << "," << real(k.id) << "*%\n";
}

void GEData::ensureAperture(RectKey k) {
  if (apRect.contains(k))
    return;
  apRect[k] = ++apidx;
  output << "%ADD" << apidx << "R," << real(k.w) << "," << real(k.h) << "*%\n";
}

void GEData::ensureAperture(SqHoleKey k) {
  if (apSqHole.contains(k))
    return;
  apSqHole[k] = ++apidx;
  output << "%ADD" << apidx << "R," << real(k.wh) << "," << real(k.wh)
	 << "," << real(k.id) << "*%\n";
}

void GEData::ensureChar(char c) {
  if (apChar.contains(c))
    return;
  apChar[c] = ++apidx;
  static SimpleFont const &sf(SimpleFont::instance());
  ensureAperture(CircKey(sf.baseLineWidth()));
  output << "G04 character " << QString::number(int(c)) << " *\n";
  output << "%ABD" << apidx << "*%\n";
  output << "G01*\n"; // linear interpolation
  selectAperture(CircKey(sf.baseLineWidth()));
  // now draw the character
  QVector<QPolygonF> strokes = sf.character(c);
  for (QPolygonF const &pp: strokes) {
    int K = pp.size();
    if (K>=2) {
      output << "X" << mil2mm(pp[0].x) << "Y" << mil2mm(pp[0].y) << "D02*\n";
      for (int k=1; k<K; k++)
	output << "X" << mil2mm(pp[k].x) << "Y" << mil2mm(pp[k].y) << "D01*\n";
    }
  }
  output << "%AB*%\n";
}

void EData::ensureCharacters(QString s) {
  int N = s.size();
  for (int n=0; n<N; n++) {
    int c = s[n];
    if (c>=32 && c<=126)
      ensureChar(c);
  }
}
  
void GEData::collectApertures(Group const &grp) {
  for (int id: grp.keys()) {
    Object const &obj(grp.object(id));
    switch (obj.type()) {
    case Object::Type::Group:
      collectApertures(obj.asGroup());
      break;
    case Object::Type::Trace:
      ensureAperture(CircKey(obj.asTrace().linewidth));
      break;
    case Object::Type::Hole:
      if (obj.asHole().square)
	ensureAperture(SqHoleKey(obj.asHole().idd, obj.asHole().od));
      else
	ensureAperture(HoleKey(obj.asHole().id, obj.asHole().od));
      ensureAperture(CircKey(obj.asHole().id));
      break;
    case Object::Type::Pad:
      ensureAperture(RectKey(obj.asPad().w, obj.asPad().h));
      break;
    case Object::Type::Text:
      ensureAperture(CircKey(fontLineWidth(obj.asText())));
      ensureCharacters(obj.asText().text);
      break;
    }
  }
}

GerberExporter::GerberExporter(QTextStream &output): d(new GEData(output)) {
}

void GerberExporter::writeComment(QString text) {
  d->output << "G04 " << text.toLatin1() << " *\n";
}

void GerberExporter::writeLayout(Layout const &layout) {
  d->output << "%FSLAX35Y35*%\n"; // specifies 5 digits past mm
  d->output << "%MOMM*%\n";

  d->collectApertures(layout.root());

  
}
