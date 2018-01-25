// FPPicture.cpp

#include "FPPicture.h"
#include <QRegularExpression>

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
public:
  bool isvalid;
  QString desc;
  QString refdes;
  QString name;
  QPicture picture;
  QPoint mxy;
  QPoint txy;
  int tdir;
  int tscale;
  int flags;
  int tflags;
  QMap<QString, PinInfo> pins;
};

void FPPicData::parseElementLine(QStringList args, QPainter &p) {
  if (args.size() != 5)
    return;
  p.setPen(QPen(QColor(0,0,0), args[4].toInt()));
  p.drawLine(QPoint(args[0].toInt(), args[1].toInt()),
	     QPoint(args[2].toInt(), args[3].toInt()));
}

void FPPicData::parseElementArc(QStringList args, QPainter &p) {
  // nyi
}

void FPPicData::parsePin(QStringList args, QPainter &p) {
  if (args.size()<5)
    return;
  int x = args.takeFirst().toInt();
  int y = args.takeFirst().toInt();
  int flags = args.takeLast().toInt(0, 0);
  int number = args.takeLast().unquote().toInt();
  QString name = args.takeLast().unquote();
  if (args.size()==9) {
    // rX rY Thickness Clearance Mask Drill "Name" "Number" NFlags
  } else if (args.size()==7) {
    // aX aY Thickness Drill "Name" "Number" NFlags
  } else if (args.size()==6) {
    // aX aY Thickness Drill "Name" NFlags
  } else if (args.size()==5) {
    // aX aY Thickness "Name" NFlags
  } else {
  }
}

void FPPicData::parseMark(QStringList args, QPainter &) {
  if (args.size() != 2)
    mxy = QPoint(args[0].toInt(), args[1].toInt());
}


FPPicture::~FPPicture() {
}

FPPicture::FPPicture(FPPicture const &o): d(o.d) {
}

FPPicture &FPPicture::operator=(FPPicture const &o) {
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
    qDebug() << "Syntax error for element" << line;
    return false;
  }

  if (!isQuoted(args.first()))
    d->flags = args.takeFirst().toInt(0, 0); // allow hex
  d->desc = unquote(args.takeFirst());
  d->refdes = unquote(args.takeFirst());
  if (isQuoted(args.first()))
    d->name = unquote(args.takeFirst());
  if (args.size()>=7) {
    int x = args.takeFirst().toInt();
    int y = args.takeFirst().toInt();
    d->mxy = QPoint(x, y);
  }
  if (args.size()<5) {
    qDebug() << "Syntax error for element" << line;
    return false;
  }
  
  int x = args.takeFirst().toInt();
  int y = args.takeFirst().toInt();
  d->txy = QPoint(x, y);
  d->tdir = args.takeFirst().toInt();
  d->tscale = args.takeFirst().toInt();
  d->tflags = args.takeFirst().toInt(0, 0); // allow hex

  return true;
}

FPPicture::FPPicture(QString fn): d(new FPPicData) {
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
		  
