// svgrecttest.cpp

#include <QSvgRenderer>
#include <QDebug>

int main() {
  QString fn("test.svg");
  QSvgRenderer rnd(fn);
  qDebug() << rnd.boundsOnElement("g7619");
  return 0;
}
