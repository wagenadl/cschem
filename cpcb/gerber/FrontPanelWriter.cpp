// FrontPanelWriter.cpp

#include "FrontPanelWriter.h"
#include "data/Layout.h"
#include "data/Object.h"
#include <QFile>
#include <QTextStream>

class FPWData {
public:
  FPWData() { }
  Rect writeSvgHeader(); // returns inferred bbox
  void writeArc(Arc const &, Point const &offset);
  void writeTrace(Trace const &, Point const &offset);
  void writeText(Text const &, Point const &offset);
  void writeGroup(Group const &, Point const &offset);
  void writeSvgFooter();
  Point inferredOffset(Group const &) const;
  Rect inferredBBox(Group const &) const;
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

Rect FPWData::inferredBBox(Group const &grp) const {
  Point offset = inferredOffset(grp);
  Rect r;
  for (int key: grp.keys()) {
    Object const &obj(grp.object(key));
    if (obj.isGroup())
      r |= inferredBBox(obj.asGroup()).translated(-offset);
    else
      r |= obj.boundingRect().translated(-offset);
  }
  return r;
}

Point FPWData::inferredOffset(Group const &grp) const {
  /* A Group has an inferrable offset if it contains a line segment that
     is not obviously connected to anything else.
     This function is not smart enough to disambiguate if there multiple
     lines, except that it will treat vertical and horizontal lines
     separately.
  */
  Dim xoffset;
  Dim yoffset;
  QMap<Point, int> concount; // counts of attachment to each point
  for (int key: grp.keys()) {
    Object const &obj(grp.object(key));
    for (Point p: obj.allPoints(Layer::Panel))
      concount[p]++;
  }
  for (int key: grp.keys()) {
    Object const &obj(grp.object(key));
    if (obj.isTrace()) {
      Trace const &trc(obj.asTrace());
      if (trc.layer==Layer::Panel
	  && concount[trc.p1]<=1
	  && concount[trc.p2]<=1) {
	if (trc.p1.x==trc.p2.x)
	  xoffset = trc.p1.x;
	else if (trc.p1.y==trc.p2.y)
	  yoffset = trc.p1.y;
      }
    }
  }
  return Point(xoffset, yoffset);
}
  

Rect FPWData::writeSvgHeader() {
  out << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n";
  out << "<svg\n";
  out << "   xmlns:dc=\"http://purl.org/dc/elements/1.1/\"\n";
  out << "   xmlns:cc=\"http://creativecommons.org/ns#\"\n";
  out << "   xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"\n";
  out << "   xmlns:svg=\"http://www.w3.org/2000/svg\"\n";
  out << "   xmlns=\"http://www.w3.org/2000/svg\"\n";
  out << "   id=\"svg8\"\n";
  out << "   version=\"1.1\"\n";

  Rect bbox = inferredBbox(layout.root());
  bbox.grow(Dim::fromInch(.25));
  out << "   viewBox=\"0 0 "
      << coord(bbox.width)
      << " "
      << coord(bbox.height)
      << "\"\n";
  out << "  " << prop("width", bbox.width) << "\n";
  out << "  " << prop("height", bbox.height) << ">\n";

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
  
  return rect;
}

void FPWData::writeSvgFooter() {
  out << "  </g>\n";
  out << "</svg>\n";
}

void FPWData::writeGroup(Group const &g, Point offset) {
  // the offset is subtracted from all objects
  offset += inferredOffset(g);
  for (int id: g.keys()) {
    Object const &obj(g.object(id));
    switch (obj.type) {
    case Object::Group:
      writeGroup(obj.asGroup(), offset);
      break;
    case Object::Trace:
      writeTrace(obj.asTrace(), offset);
      break;
    case Object::Text:
      writeText(obj.asText(), offset);
      break;
    default:
      break;
    }
  }
}

void FPWData::writeText(Text const &text, offset) {
  if (text.layer != Layer::Panel)
    return;
}

void FPWData::writeTrace(Trace const &trace, Point offset) {
  if (trace.layer != Layer::Panel)
    return;
  Point p1 = offset - trace.p1;
  Point p2 = offset - trace.p2;
  out << "    <path"
      << QString(" p=\"M %1,%2 %3,%4\"")
    .arg(coord(p1.x)).arg(coord(p1.y))
    .arg(coord(p2.x)).arg(coord(p2.y))
      << " />\n";
  return;
}

void FPWData::writeArc(Arc const &arc, Point const &offset) {
  if (arc.layer!=Layer::Panel)
    return;
  Point p = offset - arc.center;
  Dim r = arc.radius;
  out << "<circle"
      << prop("cx", p.x)
      << prop("cy", p.y)
      << prop("r", r)
      << " />\n";
}


FrontPanelWriter::FrontPanelWriter(): d(new FPWData) {
}

FrontPanelWriter::~FrontPanelWriter() {
  delete d;
}

void FrontPanelWriter::setShrinkage(Dim s) {
  d->shrinkage = s;
}

bool FrontPanelWriter::write(Layout const &layout, QString filename) {
  d->layout = layout;
  QFile file(filename);
  qDebug() << "pmw: " << filename;
  if (!file.open(QFile::WriteOnly)) 
    return false;
  qDebug() << "opened";

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
