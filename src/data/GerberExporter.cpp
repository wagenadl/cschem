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
  struct CircKey {
    explicit CircKey(Dim d): diam(d) { }
    Dim diam;
    bool operator<(CircKey const &ck) {
      return diam<ck.diam;
    }
  };
  struct HoleKey {
    HoleKey(Dim id, Dim od): id(id), od(od) { }
    Dim id;
    Dim od;
    bool operator<(HoleKey const &hk) const {
      return (id==hk.id) ? od<hk.od : id<hk.id;
    }
  };
  struct RectKey {
    RectKey(Dim w, Dim h): w(w), h(h) { }
    Dim w;
    Dim h;
    bool operator<(RectKey const &rk) const {
      return (w==rk.w) ? h<rk.h : w<rk.w;
    }
  };
  struct SqHoleKey {
    SqHoleKey(Dim id, Dim wh): id(id), wh(wh) { }
    Dim id;
    Dim wh;
    bool operator<(SqHoleKey const &rk) const {
      return (wh==rk.wh) ? id<rk.id : wh<rk.wh;
    }
  };
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
public:
  QTextStream &output;
  int apidx;
  QMap<CircKey, int> apCirc;
  QMap<RectKey, int> apRect;
  QMap<HoleKey, int> apHole;
  QMap<SqHoleKey, int> apSqHole;
  QMap<char, int> amChar;
};

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
  ensureAperture(sf.baseLineWidth());
  // now draw the character into a macro
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
  d->outputApertures();
}
