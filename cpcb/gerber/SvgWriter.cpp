// SvgWriter.cpp

#include "SvgWriter.h"

SvgWriter::SvgWriter(QString filename, Dim width, Dim height): file(filename) {
  file.open(QFile::WriteOnly | QFile::Text);
  stream = QTextStream(&file);
  writeHeader(width, height);
}

SvgWriter::~SvgWriter() {
  writeFooter();
  stream = QTextStream(); // call the stream's destructor first
  file = QFile(); // then the file's destructor
}

bool SvgWriter::isValid() const {
  return file.error() == QFile::NoError;
}

SvgWriter &SvgWriter::operator<<(QString s) {
  stream << s;
  return *this;
}

QString SvgWriter::coord(Dim const &x) {
  return QString::number(x.toInch()*96, 'f', 5);
}

QString SvgWriter::prop(QString name, Dim const &x) {
  return QString(" " + name + "=\"" + coord(x) + "\"");
}  

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

QString hexbyte(int x) {
  return QString::number(x, 2, 16, '0');
}

QString SvgWriter::color(QColor c) {
  return "#" + hexbyte(c.red()) + hexbyte(c.green()) + hexbyte(c.blue());
}

void SvgWriter::drawRect(Rect const &rect, QColor const &c,
                         Dim width) {
  stream << "   <rect"
         << prop("x", rect.left)
         << prop("y", rect.top)
         << prop("width", rect.width) 
         << prop("height", rect.height)
         << " style=" << '"'
         << "opacity:1;"
         << "fill:none;"
         << "stroke:" << color(c) << ";"
         << "stroke-width:" << dim(width)
         << '"' << " />\n";    
}

void SvgWriter::fillRect(Rect const &rect, QColor color) {
  stream << "   <rect"
         << prop("x", rect.left)
         << prop("y", rect.top)
         << prop("width", rect.width) 
         << prop("height", rect.height)
         << " style=" << '"'
         << "opacity:1;"
         << "fill:" << color(c) << ";"
         << "stroke:none"
         << '"' << " />\n";    
}

void SvgWriter::fillRing(Point const &center, Dim inner, Dim outer,
                        QColor const &color) {
  QString ro = coord(outer);
  QString ri = coord(inner);
  stream << "    <path"
         << " style=" << '"'
         << "opacity:1;"
         << "fill:" << color(c) << ";"
         << "stroke:none"
         << '"';
  stream << " d=" << '"'
         << "M " << coord(center.x) << "," << coord(center.y)
         << " m " << "0,-" << ro
         << " a " << ro << "," << ro << " 0 0 0 -" << ro << "," << ro
         << " "  << ro << "," << ro << " 0 0 0 " << ro << "," << ro
         << " "  << ro << "," << ro << " 0 0 0 " << ro << ",-" << ro
         << " "  << ro << "," << ro << " 0 0 0 -" << ro << ",-" << ro
         << " z"
         << " M " << coord(center.x) << "," << coord(center.y)
         << " m " << "0,-" << ri
         << " a " << ri << "," << ri << " 0 0 1 " << ri << "," << ri
         << " "  << ri << "," << ri << " 0 0 1 -" << ri << "," << ri
         << " "  << ri << "," << ri << " 0 0 1 -" << ri << ",-" << ri
         << " "  << ri << "," << ri << " 0 0 1 " << ri << ",-" << ri
         << " z"
         << '"';
    stream << "/>";
}

QString quote(QString text) {
  return text.replace("&", "&amp;")
    .replace("<", "&lt;")
    .replace(">", "&gt;")
    .replace("\"", "&quot;");
}  

void SvgWriter::drawText(QString text, Point const &anchor, QColor const &color,
                         Dim fontsize, int angle_degcw) {
  stream << "    <g transform=\""
         << "translate(" << coord(anchor.x) << "," << coord(anchor.y) << "),"
         << "rotate(" << angle_degcw << "),"
         << "translate(0,-" << coord(fontsize/3) << ")\">";
  stream << "    <text"
         << " style=" << '"'
         << "font-size:" << coord(fontsize) << "px;"
         << "font-family:Helvetica;"
         << "text-align:center;text-anchor:middle;"
         << "opacity:1;"
         << "fill:" << color(c) << ";"
         << "stroke:none;"
         << "\">";
  stream << quote(text);
  stream << "</text>";
  stream << "</g>";
}
