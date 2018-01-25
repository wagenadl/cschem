#include <QRegularExpression>
#include <QDebug>
#include <QFile>

int main() {
  QFile f("/usr/share/pcb/pcblib-newlib/geda/ACY100.fp");
  if (!f.open(QFile::ReadOnly))
    return 1 ;
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
      qDebug() << "got" << kwd << "and" << sub;
      QRegularExpression re("([^\\s\"]+)|(\"[^\"]+\")");
      QStringList args;
      auto gm = re.globalMatch(sub);
      while (gm.hasNext())
	args << gm.next().captured(0);
      qDebug() << "split" << args;
      
    }
  }
  return 0;
}
