// PackageDrawing.cpp

#include "PackageDrawing.h"

#include <QFile>
#include <QPainter>
#include <QRegularExpression>
#include <QDebug>

class FPPicData {
public:
  FPPicData() { isvalid = false; }
  static bool isQuoted(QString x);
  static QString unquote(QString x);
  bool parseElement(QStringList args);
  void parseElementLine(QStringList args, QPainter &);
  void parseElementArc(QStringList args, QPainter &);
  void parsePin(QStringList args, QPainter &);
  void parseMark(QStringList args, QPainter &);
  static QColor silkColor() { return QColor(0, 0, 255); }
  static QColor holeColor() { return QColor(255, 255, 255); }
  static QColor padColor() { return QColor(0, 0, 0); }
public:
  bool isvalid;
  QString desc;
  QString refdes;
  QString name;
  QPicture picture;
  QPoint markpos;
  QPoint textpos;
  int tdir;
  int tscale;
  int flags;
  int tflags;
  QMap<int, PackageDrawing::PinInfo> pins; // organized by number
};

void FPPicData::parseElementLine(QStringList args, QPainter &p) {
  if (args.size() != 5)
    return;
  p.setPen(QPen(silkColor(), args[4].toInt()));
  p.drawLine(QPoint(args[0].toInt(), args[1].toInt()),
	     QPoint(args[2].toInt(), args[3].toInt()));
}

void FPPicData::parseElementArc(QStringList, QPainter &) {
  qDebug() << "FPPicData::parseElementArc: NYI";
  // nyi
}

void FPPicData::parsePin(QStringList args, QPainter &p) {
  if (args.size()<5)
    return;
  PackageDrawing::PinInfo pin;
  int x = args.takeFirst().toInt();
  int y = args.takeFirst().toInt();
  pin.position = QPoint(x, y);
  pin.padDiameter = args.takeFirst().toInt();
  QString flags = args.takeLast();
  if (isQuoted(flags)) 
    pin.isSquare = unquote(flags).split(",").contains("square");
  else
    pin.isSquare = flags.toInt() & 0x0100;
  QString nn = unquote(args.takeLast());
  pin.number = nn.toInt();
  if (args.size()==4) {
    // [rX rY Thickness] Clearance Mask Drill "Name" ["Number" NFlags]
    // Let's hope we already have a mark, otherwise, we're going to fail
    pin.position += markpos;
    pin.clearanceDiameter = args.takeFirst().toInt();
    pin.drillDiameter = args.takeFirst().toInt();
    pin.name = unquote(args.takeFirst());
  } else if (args.size()==2) {
    // [aX aY Thickness] Drill "Name" ["Number" NFlags]
    pin.drillDiameter =  args.takeFirst().toInt();
    pin.name = unquote(args.takeFirst());
  } else if (args.size()==1) {
    // [aX aY Thickness] Drill ["Name" NFlags]
    pin.drillDiameter =  args.takeFirst().toInt();
    pin.name = nn;
  } else if (args.size()==0) {
    // [aX aY Thickness] ["Name" NFlags]
    pin.name = nn;
  } else {
    return;
  }
  pin.clearanceDiameter += pin.padDiameter;

  p.setBrush(QBrush(padColor()));
  p.setPen(Qt::NoPen);
  if (pin.isSquare)
    p.drawRect(QRect(pin.position, QSize(pin.padDiameter, pin.padDiameter))
		    .translated(QPoint(pin.padDiameter/2, pin.padDiameter/2)));
  else
    p.drawEllipse(pin.position, pin.padDiameter/2, pin.padDiameter/2);

  if (pin.drillDiameter>0) {
    p.setBrush(QBrush(holeColor()));
    p.drawEllipse(pin.position, pin.drillDiameter/2, pin.drillDiameter/2);
  }  
}

void FPPicData::parseMark(QStringList args, QPainter &) {
  if (args.size() != 2)
    markpos = QPoint(args[0].toInt(), args[1].toInt());
}


PackageDrawing::~PackageDrawing() {
}

PackageDrawing::PackageDrawing(PackageDrawing const &o): d(o.d) {
}

PackageDrawing &PackageDrawing::operator=(PackageDrawing const &o) {
  d = o.d;
  return *this;
}


bool FPPicData::isQuoted(QString x) {
  return x.startsWith("\"") && x.endsWith("\"");
}

QString FPPicData::unquote(QString x) {
  // removes quotes and also trailing comma
  if (isQuoted(x))
    x = x.mid(1, x.size() - 2).trimmed();
  if (x.endsWith(","))
    x = x.left(x.size()-1).trimmed();
  return x;
}

bool FPPicData::parseElement(QStringList args) {
  if (args.size()<7) {
    qDebug() << "Syntax error for element" << args.join(" ");
    return false;
  }

  if (!isQuoted(args.first()))
    flags = args.takeFirst().toInt(0, 0); // allow hex
  desc = unquote(args.takeFirst());
  refdes = unquote(args.takeFirst());
  if (isQuoted(args.first()))
    name = unquote(args.takeFirst());
  if (args.size()>=7) {
    int x = args.takeFirst().toInt();
    int y = args.takeFirst().toInt();
    markpos = QPoint(x, y);
  }
  if (args.size()<5) {
    qDebug() << "Syntax error for element" << args.join(" ");
    return false;
  }
  
  int x = args.takeFirst().toInt();
  int y = args.takeFirst().toInt();
  textpos = QPoint(x, y);
  tdir = args.takeFirst().toInt();
  tscale = args.takeFirst().toInt();
  tflags = args.takeFirst().toInt(0, 0); // allow hex

  return true;
}

PackageDrawing::PackageDrawing(QString fn): d(new FPPicData) {
  QFile f(fn);
  if (!f.open(QFile::ReadOnly))
    return;

  QPainter p;
  p.begin(&d->picture);
  
  QTextStream ts(&f);
  while (!ts.atEnd()) {
    QString line = ts.readLine().replace(QRegularExpression("#.*"), "")
      .simplified();
    if (line.isEmpty())
      continue;
    line.replace("[", "(");
    line.replace("]", "]");
    QRegularExpression re("(.*?)\\s*\\(\\s*(.*)\\s*\\)");
    // that is KEYWORD(ARG1 ARG2...)
    auto match = re.match(line);
    if (match.hasMatch()) {
      QString kwd = match.captured(1);
      QString sub = match.captured(2);
      QRegularExpression re("([^\\s\"]+)|(\"[^\"]+\")");
      QStringList args;
      auto gm = re.globalMatch(sub);
      while (gm.hasNext())
	args << gm.next().captured(0);

      if (kwd=="Element") {
	if (!d->parseElement(args))
	  return; // this won't work
      } else if (kwd=="Pin") {
	d->parsePin(args, p);
      } else if (kwd=="ElementLine") {
	d->parseElementLine(args, p);
      } else if (kwd=="ElementArc") {
	d->parseElementLine(args, p);
      } else if (kwd=="Mark") {
	d->parseMark(args, p);
      }
    } else if (line=="(") {
      // proceed into contents
    } else if (line==")") {
      break;
    }
  }
  p.end();
}
		  