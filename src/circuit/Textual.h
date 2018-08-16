// Textual.h

#ifndef TEXTUAL_H

#define TEXTUAL_H

#include <QPoint>
#include <QXmlStreamReader>
#include <QDebug>

class Textual {
public:
  Textual(QPoint p=QPoint(), QString txt="");
  QString report() const;
  Textual translated(QPoint delta) const;
public:
  int id;
  QPoint position;
  QString text;
};

QXmlStreamWriter &operator<<(QXmlStreamWriter &, Textual const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Textual &);
QDebug &operator<<(QDebug &, Textual const &);

#endif
