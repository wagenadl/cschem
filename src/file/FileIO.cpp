// FileIO.cpp

#include "FileIO.h"
#include <QDebug>
#include <QFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

namespace FileIO {

  Schem loadSchematic(QString fn) {
    QFile file(fn);
    if (file.open(QFile::ReadOnly)) {
      QXmlStreamReader sr(&file);
      while (!sr.atEnd()) {
        sr.readNext();
        if (sr.isStartElement() && sr.name() == "qschem")
          return Schem(sr);
      }
    }
    qDebug() << "Failed to load" << fn;
    return Schem();
  }
  
  void saveSchematic(QString fn, Schem const &s) {
    QFile file(fn);
    if (file.open(QFile::WriteOnly)) {
      QXmlStreamWriter sw(&file);
      sw.setAutoFormatting(true);
      sw.setAutoFormattingIndent(2);
      sw.writeStartDocument("1.0", false);
      sw << s;
      sw.writeEndDocument();
    } else {
      qDebug() << "Failed to save" << fn;
    }
  }    
};
  
