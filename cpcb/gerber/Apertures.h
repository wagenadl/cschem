// Apertures.h

#ifndef APERTURES_H

#define APERTURES_H

#include "data/Dim.h"
#include <QMap>
#include <QTextStream>

namespace Gerber {
  struct Circ {
    explicit Circ(Dim d): diam(d) { }
    Dim diam;
    bool operator<(Circ const &ck) const {
      return diam<ck.diam;
    }
  };
  struct Hole {
    Hole(Dim id, Dim od): id(id), od(od) { }
    Dim id;
    Dim od;
    bool operator<(Hole const &hk) const {
      return (id==hk.id) ? od<hk.od : id<hk.id;
    }
  };
  struct Rect {
    Rect(Dim w0, Dim h0=Dim()): w(w0), h(h0) { if (h.isNull()) h = w; }
    Dim w;
    Dim h;
    bool operator<(Rect const &rk) const {
      return (w==rk.w) ? h<rk.h : w<rk.w;
    }
  };
  struct SqHole {
    SqHole(Dim id, Dim wh): id(id), wh(wh) { }
    Dim id;
    Dim wh;
    bool operator<(SqHole const &rk) const {
      return (wh==rk.wh) ? id<rk.id : wh<rk.wh;
    }
  };

  class Apertures {
  public:
    enum class Func {
      Invalid,
      Material,
      Conductor,
      NonConductor,
      Profile,
      ComponentDrill,
      ComponentPad,
      SMDPad,
      AntiPad,
      // HeatsinkPad,
      // ViaDrill,
      // ViaPad,
    };
    static QString funcName(Func);
  public:
    Apertures(Func func=Func::Invalid, int firstidx=10);
    Func func() const;
    int nextIndex() const { return apidx; }
    int firstIndex() const;
    void write(QTextStream &output) const;
    void ensure(Circ);
    void ensure(Rect);
    void ensure(Hole);
    void ensure(SqHole);
    QString select(Circ) const;
    QString select(Rect) const;
    QString select(Hole) const;
    QString select(SqHole) const;
  private:
    Func func_;
    int apidx;
    QMap<Circ, int> apCirc;
    QMap<Rect, int> apRect;
    QMap<Hole, int> apHole;
    QMap<SqHole, int> apSqHole;
  };

  QTextStream &operator<<(QTextStream &ts, Apertures &ap);

};

#endif

