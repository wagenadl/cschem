// Aperture.h

#ifndef APERTURE_H

#define APERTURE_H

#include "data/Dim.h"
#include <QMap>
#include <QTextStream>

namespace Gerber {
  struct Circ {
    explicit Circ(Dim d): diam(d) { }
    Dim diam;
    bool operator<(Circ const &ck) {
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
    Rect(Dim w, Dim h): w(w), h(h) { }
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

  class Aperture {
  public:
    Aperture();
    void write(QTextStream &output);
    void ensure(Circ);
    void ensure(Rect);
    void ensure(Hole);
    void ensure(SqHole);
    QString select(Circ) const;
    QString select(Rect) const;
    QString select(Hole) const;
    QString select(SqHole) const;
  private:
    int apidx;
    QMap<Circ, int> apCirc;
    QMap<Rect, int> apRect;
    QMap<Hole, int> apHole;
    QMap<SqHole, int> apSqHole;
  };

};

#endif

