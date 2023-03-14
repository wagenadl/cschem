// PCBFileIO.cpp

#include "PCBFileIO.h"
#include <QDebug>
#include <QFile>
#include <QXmlStreamReader>

namespace PCBFileIO {

  Layout loadLayout(QString fn) {
    QFile file(fn);
    if (file.open(QFile::ReadOnly)) {
      QXmlStreamReader sr(&file);
      while (!sr.atEnd()) {
        sr.readNext();
        if (sr.isStartElement() && sr.name() == "cpcb") {
          Layout l;
	  sr >> l;
	  return l;
	}
      }
    }
    qDebug() << "Failed to load" << fn;
    return Layout();
  }
  
  bool saveLayout(QString fn, Layout const &s) {
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
  
