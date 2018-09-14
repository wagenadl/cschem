// PasteMaskWriter.cpp

#include "PasteMaskWriter.h"
#include "data/Layout.h"
#include "data/Object.h"
#include <QFile>
#include <QTextStream>

class PMWData {
public:
  PMWData() { }
  bool writeSvgHeader();
  bool writeSvgOutline();
  bool writeSvgPads();
  bool writePad(Pad const &);
  bool writeGroup(Group const &);
  bool writeSvgFooter();
  QString coord(Dim x) {
    return QString::number(x.toInch()*96, 'f', 5);
  }
  QString prop(QString name, Dim x) {
    return QString(" " + name + "=\"" + coord(x) + "\"");
  }  
public:
  Layout layout;
  QTextStream out;
  Dim shrinkage;
};

bool PMWData::writeSvgHeader() {
  out << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n";
  out << "<svg\n";
  out << "   xmlns:dc=\"http://purl.org/dc/elements/1.1/\"\n";
  out << "   xmlns:cc=\"http://creativecommons.org/ns#\"\n";
  out << "   xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"\n";
  out << "   xmlns:svg=\"http://www.w3.org/2000/svg\"\n";
  out << "   xmlns=\"http://www.w3.org/2000/svg\"\n";
  out << "   id=\"svg8\"\n";
  out << "   version=\"1.1\"\n";

  out << "   viewBox=\"0 0 "
      << coord(layout.board().width)
      << " "
      << coord(layout.board().height)
      << "\"\n";
  out << "    " << prop("width", layout.board().width) << "\n";
  out << "    " << prop("height", layout.board().height) << "\n";

  out << "  <defs\n";
  out << "     id=\"defs2\" />\n";
  out << "  <metadata\n";
  out << "     id=\"metadata5\">\n";
  out << "    <rdf:RDF>\n";
  out << "      <cc:Work\n";
  out << "         rdf:about=\"\">\n";
  out << "        <dc:format>image/svg+xml</dc:format>\n";
  out << "        <dc:type "
      << "rdf:resource=\"http://purl.org/dc/dcmitype/StillImage\" />\n";
  out << "        <dc:title></dc:title>\n";
  out << "      </cc:Work>\n";
  out << "    </rdf:RDF>\n";
  out << "  </metadata>\n";
  out << "  <g\n";
  out << "     id=\"layer1\">\n";
  
  return true;
}

bool PMWData::writeSvgFooter() {
  return true;
}
  
bool PMWData::writeSvgOutline() {
  out << "   <rect"
      << prop("x", Dim::fromInch(0))
      << prop("y", Dim::fromInch(0))
      << prop("width", layout.board().width) 
      << prop("height", layout.board().height)
      << " style=\"opacity:1;fill:none;stroke:#ff0000;stroke-width:1\""
      << " />\n";    
  return true;
}

bool PMWData::writeGroup(Group const &g) {
  for (int id: g.keys()) {
    Object const &obj(g.object(id));
    if (obj.isGroup())
      writeGroup(obj.asGroup());
    else if (obj.isPad())
      writePad(obj.asPad());
  }
  return true;
}

bool PMWData::writePad(Pad const &pad) {
  if (pad.layer != Layer::Top)
    return true; // don't do anything
  Dim x0 = pad.p.x - pad.width/2 + shrinkage;
  Dim y0 = pad.p.y - pad.height/2 + shrinkage;
  Dim w = pad.width - 2*shrinkage;
  Dim h = pad.height - 2*shrinkage;
  out << "    <rect"
      << prop("x", x0)
      << prop("y", y0)
      << prop("width", w)
      << prop("height", h)
      << " style=\"opacity:1;fill:#000000;stroke:none\""
      << " />\n";
  return true;
}
  
bool PMWData::writeSvgPads() {
  return writeGroup(layout.root());
}

PasteMaskWriter::PasteMaskWriter(): d(new PMWData) {
}

PasteMaskWriter::~PasteMaskWriter() {
  delete d;
}

void PasteMaskWriter::setShrinkage(Dim s) {
  d->shrinkage = s;
}

bool PasteMaskWriter::write(Layout const &layout, QString filename) {
  d->layout = layout;
  QFile file(filename);
  if (file.open(QFile::WriteOnly)) 
    return false;

  d->out.setDevice(&file);
  
  if (!d->writeSvgHeader())
    return false;
  if (!d->writeSvgOutline())
    return false;
  if (!d->writeSvgPads())
    return false;
  if (!d->writeSvgFooter())
    return false;

  d->out.setDevice(0);
  file.close();

  return true;
}
