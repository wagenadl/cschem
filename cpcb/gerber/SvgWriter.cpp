// SvgWriter.cpp

#include "SvgWriter.h"
#include "data/pi.h"

static QString coord(Dim const &x) {
  return QString::number(x.toInch()*96, 'f', 5);
}

static QString point(Point const &p) {
  return coord(p.x) + "," + coord(p.y);
}

static QString prop(QString name, Dim const &x) {
  return QString(" " + name + "=\"" + coord(x) + "\"");
}


static QString hexbyte(int x) {
  return QString("%1").arg(x, 2, 16, QChar('0'));
}

static QString color(QColor const &c) {
  return "#" + hexbyte(c.red()) + hexbyte(c.green()) + hexbyte(c.blue());
}

//////////////////////////////////////////////////////////////////////
class SvgPathData {
public:
  SvgPathData();
  void moveTo(Point const &p, bool relative);
  void lineTo(Point const &p, bool relative);
  void arcTo(Point const &p, Dim const &radius, bool longest, bool ccw, bool relative);
  void close();
  QString toString() const { return pth; };
  QString toProp() const { return " d=\"" + pth + "\""; }
private:
  QString pth;
};

SvgPathData::SvgPathData() {
}

void SvgPathData::moveTo(Point const &p, bool relative) {
  pth += relative ? " m" : " M";
  pth += point(p);
}

void SvgPathData::lineTo(Point const &p, bool relative) {
  pth += relative ? " l" : " L";
  pth += point(p);
}

void SvgPathData::arcTo(Point const &p, Dim const &radius,
                        bool longest, bool ccw, bool relative) {
  pth += relative ? " a": " A";
  pth += coord(radius) + "," + coord(radius);
  pth += " 0 ";
  pth += longest ? "1" : "0";
  pth += ccw ? " 1 " : " 0 ";
  pth += point(p);
}

void SvgPathData::close() {
  pth += "Z";
}

//////////////////////////////////////////////////////////////////////

class SvgStyle {
public:
  SvgStyle(): hasstroke(false), hasfill(false), hasopac(false) {}
  void opacity(float);
  void fill(QColor c);
  void stroke(QColor c, Dim width, QString cap="round");
  void font(QString family, Dim size);
  QString toString() const;
  QString toProp() const { return " style=\"" + toString() + "\""; }
private:
  QString s;
  bool hasstroke, hasfill, hasopac;
};

QString SvgStyle::toString() const {
  QString s1 = s;
  if (!hasstroke)
    s1 += " stroke:none;";
  if (!hasfill)
    s1 += " fill:none;";
  if (!hasopac)
    s1 += " opacity:1;";
  return s1;
}

void SvgStyle::font(QString family, Dim size) {
  s += " font-family:" + family + ";";
  s += " font-size:" + coord(size) + "px;";
}

void SvgStyle::opacity(float o) {
  s +=" opacity:" + QString::number(o) + ";";
  hasopac = true;
}

void SvgStyle::fill(QColor c) {
  s += " fill:" + color(c) + ";";
  hasfill = true;
}

void SvgStyle::stroke(QColor c, Dim width, QString cap) {
  s += " stroke:" + color(c) + ";";
  s += " stroke-width:" + coord(width) + ";";
  s += " stroke-linecap:" + cap + ";";
  hasstroke = true;
} 

//////////////////////////////////////////////////////////////////////  



SvgWriter::SvgWriter(QFile *file, Dim width, Dim height): stream(file) {
  writeHeader(width, height);
}

SvgWriter::~SvgWriter() {
  writeFooter();
}

SvgWriter &SvgWriter::operator<<(QString s) {
  stream << s;
  return *this;
}

//QString SvgWriter::color(QColor const &x) { return ::color(x); }
//QString SvgWriter::coord(Dim const &x) { return ::coord(x); }
//QString SvgWriter::prop(QString name, Dim const &x) { return ::prop(name, x); }


void SvgWriter::writeHeader(Dim w, Dim h) {
  stream << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n";
  stream << "<svg\n";
  stream << "   xmlns:dc=\"http://purl.org/dc/elements/1.1/\"\n";
  stream << "   xmlns:cc=\"http://creativecommons.org/ns#\"\n";
  stream << "   xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"\n";
  stream << "   xmlns:svg=\"http://www.w3.org/2000/svg\"\n";
  stream << "   xmlns=\"http://www.w3.org/2000/svg\"\n";
  stream << "   id=\"svg8\"\n";
  stream << "   version=\"1.1\"\n";

  stream << "   viewBox=\"0 0 " << coord(w) << " " << coord(h) << "\"\n";
  stream << "  " << prop("width", w) << "\n";
  stream << "  " << prop("height", h) << ">\n";

  stream << "  <defs\n";
  stream << "     id=\"defs2\" />\n";
  stream << "  <metadata\n";
  stream << "     id=\"metadata5\">\n";
  stream << "    <rdf:RDF>\n";
  stream << "      <cc:Work\n";
  stream << "         rdf:abstream=\"\">\n";
  stream << "        <dc:format>image/svg+xml</dc:format>\n";
  stream << "        <dc:type "
      << "rdf:resource=\"http://purl.org/dc/dcmitype/StillImage\" />\n";
  stream << "        <dc:title></dc:title>\n";
  stream << "      </cc:Work>\n";
  stream << "    </rdf:RDF>\n";
  stream << "  </metadata>\n";
  stream << "  <g\n";
  stream << "     id=\"layer1\">\n";
}

void SvgWriter::writeFooter() {
  stream << "  </g>\n";
  stream << "</svg>\n";
}

void SvgWriter::drawSegment(Point const &p1, Point const &p2, Dim const &width,
                            QColor const &c) {
  SvgStyle s;
  s.stroke(c, width);
  stream << "   <line"
         << prop("x1", p1.x)
         << prop("y1", p1.y)
         << prop("x2", p2.x)
         << prop("y2", p2.y)
         << s.toProp()
         << "/>\n";    
}

void SvgWriter::drawRect(Rect const &rect, QColor const &c,
                         Dim width) {
  
  SvgStyle s;
  s.stroke(c, width);
  stream << "   <rect"
         << prop("x", rect.left)
         << prop("y", rect.top)
         << prop("width", rect.width) 
         << prop("height", rect.height)
         << s.toProp()
         << "/>\n";    
}

void SvgWriter::fillRect(Rect const &rect, QColor const &c) {
  SvgStyle s;
  s.fill(c);
  stream << "   <rect"
         << prop("x", rect.left)
         << prop("y", rect.top)
         << prop("width", rect.width) 
         << prop("height", rect.height)
         << s.toProp()
         << "/>\n";    
}

void SvgWriter::drawCircle(Point const &center, Dim const &diam, QColor const &c,
                           Dim width) {
  
  SvgStyle s;
  s.stroke(c, width);
  stream << "   <circle"
         << prop("cx", center.x)
         << prop("cy", center.y)
         << prop("r", diam/2) 
         << s.toProp()
         << "/>\n";    
}

void SvgWriter::fillCircle(Point const &center, Dim const &diam, QColor const &c) {
  
  SvgStyle s;
  s.fill(c);
  stream << "   <circle"
         << prop("cx", center.x)
         << prop("cy", center.y)
         << prop("r", diam/2) 
         << s.toProp()
         << "/>\n";    
}

/*
void SvgWriter::fillRing(Point const &center, Dim inner, Dim outer,
                         QColor const &c) {
  //  QString ro = coord(outer/2);
  //  QString ri = coord(inner/2);
  SvgStyle s;
  s.fill(c);
  SvgPathData pd;
  Point po(outer, Dim());
  Point pi(inner, Dim()); 
  pd.moveTo(center, false);
  pd.moveTo(po/2, true);
  pd.arcTo(-po, outer/2, true, false, true);
  pd.arcTo(po, outer/2, true, false, true);
  pd.moveTo(pi/2 - po/2, true);
  pd.arcTo(-pi, inner/2, true, true, true);
  pd.arcTo(pi, inner/2, true, true, true);
  pd.close();
  stream << "    <path"
         << s.toProp()
         << pd.toProp()
         << "/>\n";
}
*/

QString quote(QString text) {
  return text.replace("&", "&amp;")
    .replace("<", "&lt;")
    .replace(">", "&gt;")
    .replace("\"", "&quot;");
}  

void SvgWriter::drawText(QString text, Point const &anchor, QColor const &c,
                         Dim fontsize, double angle_degcw) {
  startGroup(anchor, angle_degcw);
  startGroup(Point(-fontsize/3, fontsize/3), 0);
  SvgStyle s;
  s.fill(c);
  s.font("Helvetica", fontsize);
  stream << "      <text"
         << s.toProp()
         << ">"
         << quote(text)
         << "</text>\n";
  endGroup();
  endGroup();
}

void SvgWriter::startGroup(Point const &translate, double angle_degcw) {
  stream << "<g transform=\"translate(" << point(translate) << "),"
         << "rotate(" << angle_degcw << ")\">";
}

void SvgWriter::endGroup() {
  stream << "</g>\n";
}

//////////////////////////////////////////////////////////////////////

void SvgWriter::renderTrace(Trace const &trace) {
  drawSegment(trace.p1, trace.p2, trace.width, layerColor(trace.layer));
}
 
void SvgWriter::renderHole(Hole const &hole) {
  if (hole.slotlength == Dim()
      && (!hole.square || fmod(hole.rota.degrees(), 90) == 0)) {
    // simple case, save some complexity in svg output
    if (hole.square) {
      Point dp(hole.od, hole.od);
      fillRect(Rect(hole.p - dp/2, hole.p + dp/2), layerColor(Layer::Top));
    } else {
      fillCircle(hole.p, hole.od, layerColor(Layer::Top));
    }
    fillCircle(hole.p, hole.id, QColor(128, 128, 128));    
  } else {
    // case with elongation or rotated square
    startGroup(hole.p, hole.rota.degrees());
    if (hole.square) {
      Point dp(hole.od + hole.slotlength, hole.od);
      fillRect(Rect(-dp/2, dp/2), layerColor(Layer::Top));
    } else {
      SvgStyle s;
      s.fill(layerColor(Layer::Top));
      SvgPathData pd;
      pd.moveTo(Point(-hole.slotlength/2, -hole.od/2), false);
      pd.arcTo(Point(Dim(), hole.od), hole.od/2, false, false, true);
      pd.lineTo(Point(hole.slotlength, Dim()), true);
      pd.arcTo(Point(Dim(), -hole.od), hole.od/2, false, false, true);
      pd.lineTo(Point(-hole.slotlength, Dim()), true);
      pd.close();
      stream << "<path"
             << s.toProp()
             << pd.toProp()
             << "/>\n";
    };
    SvgStyle s;
    s.fill(QColor(128, 128, 128));
    SvgPathData pd;
    pd.moveTo(Point(-hole.slotlength/2, -hole.id/2), false);
    pd.arcTo(Point(Dim(), hole.id), hole.id/2, false, false, true);
    pd.lineTo(Point(hole.slotlength, Dim()), true);
    pd.arcTo(Point(Dim(), -hole.id), hole.id/2, false, false, true);
    pd.lineTo(Point(-hole.slotlength, Dim()), true);
    pd.close();
    stream << "<path"
           << s.toProp()
           << pd.toProp()
           << "/>\n";
    endGroup();
  }
}
 
void SvgWriter::renderPad(Pad const &pad) {
  startGroup(pad.p, pad.rota.degrees());
  Point dp(pad.width, pad.height);
  fillRect(Rect(-dp/2, dp/2), layerColor(pad.layer));
  endGroup();
}

void SvgWriter::renderArc(Arc const &arc) {
  if (arc.angle >= 360) {
    drawCircle(arc.center, arc.radius*2, layerColor(arc.layer), arc.linewidth);
  } else {
    SvgStyle s;
    s.stroke(layerColor(arc.layer), arc.linewidth);
    double a1 = arc.rota.degrees() - 90;
    double a2 = a1 + arc.angle;
    SvgPathData pd;
    Point dp1(arc.radius*cos(a1 * PI / 180), arc.radius*sin(a1 * PI / 180));
    Point dp2(arc.radius*cos(a2 * PI / 180), arc.radius*sin(a2 * PI / 180));
    pd.moveTo(arc.center + dp2, false);
    pd.arcTo(dp1 - dp2, arc.radius, arc.angle >= 180, false, true);
    stream << "<path"
           << s.toProp()
           << pd.toProp()
           << "/>\n";
  }
}

void SvgWriter::renderNPHole(NPHole const &hole) {
  if (hole.slotlength == Dim()) {
    fillCircle(hole.p, hole.d, QColor(128, 128, 128));
  } else {
    startGroup(hole.p, hole.rota.degrees());
    SvgStyle s;
    s.fill(QColor(128, 128, 128));
    SvgPathData pd;
    pd.moveTo(Point(-hole.slotlength/2, -hole.d/2), false);
    pd.arcTo(Point(Dim(), hole.d), hole.d/2, false, false, true);
    pd.lineTo(Point(hole.slotlength, Dim()), true);
    pd.arcTo(Point(Dim(), -hole.d), hole.d/2, false, false, true);
    pd.lineTo(Point(-hole.slotlength, Dim()), true);
    pd.close();
    stream << "<path"
           << s.toProp()
           << pd.toProp()
           << "/>\n";
    endGroup();
  }
}

void SvgWriter::renderPlane(FilledPlane const &plane) {
}


