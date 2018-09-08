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
        if (sr.isStartElement() && sr.name() == "cschem")
          return Schem(sr);
      }
    }
    qDebug() << "Failed to load" << fn;
    return Schem(false);
  }
  
  bool saveSchematic(QString fn, Schem const &s) {
    QFile file(fn);
    if (file.open(QFile::WriteOnly)) {
      QXmlStreamWriter sw(&file);
      sw.setAutoFormatting(true);
      sw.setAutoFormattingIndent(2);
      sw.writeStartDocument("1.0", false);
      sw << s;
      sw.writeEndDocument();
      return true;
    } else {
      qDebug() << "Failed to save" << fn;
      return false;
    }
  }    
};
  
