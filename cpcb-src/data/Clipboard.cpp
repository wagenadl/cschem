// Clipboard.cpp

#include "Clipboard.h"
#include <QApplication>
#include <QClipboard>
#include <QBuffer>
#include <QXmlStreamWriter>
#include <QMimeData>

Clipboard &Clipboard::instance() {
  static Clipboard cb;
  return cb;
}

Clipboard::Clipboard() {
  // we have no static information; we actually store everything in the
  // system clipboard
}

bool Clipboard::isValid() const {
  QClipboard *cb = QApplication::clipboard();
  QMimeData const *md = cb->mimeData();
  return md->hasFormat(dndformat);
}

void Clipboard::store(Group const &root, QSet<int> selection) {
  qDebug() << "Storing" << selection.size() << "objects";
  QBuffer buf;
  buf.open(QBuffer::WriteOnly);
  { QXmlStreamWriter s(&buf);
    s.writeStartDocument("1.0", false);
    s.writeStartElement("cpcbsel");
    s.writeDefaultNamespace("http://www.danielwagenaar.net/cpcbsel-ns.html");
    for (int id: selection)
      if (root.contains(id))
	s << root.object(id);
    s.writeEndElement();
    s.writeEndDocument();
  }
  buf.close();
  QClipboard *cb = QApplication::clipboard();
  QMimeData *md = new QMimeData;
  md->setData(dndformat, buf.buffer());
  qDebug() << "Storing" << QString(buf.buffer());
  cb->setMimeData(md);
}

QList<Object> Clipboard::retrieve() const {
  QList<Object> lst;
  if (!isValid())
    return lst;
  QClipboard *cb = QApplication::clipboard();
  QMimeData const *md = cb->mimeData();
  QByteArray ba(md->data(dndformat));
  QBuffer buf(&ba);
  qDebug() << "Retrieving" << QString(buf.buffer());
  if (!buf.open(QBuffer::ReadOnly)) {
    qDebug() << "Failed to open clipboard buffer";
    return lst;
  }
  QXmlStreamReader s(&buf);
  while (!s.atEnd()) {
    s.readNext();
    if (s.isStartElement() && s.name()=="cpcbsel") {
      while (!s.atEnd()) {
	s.readNext();
	if (s.isStartElement()) {
	  Object o;
	  s >> o;
	  qDebug() << "Retrieved" << o;
	  lst << o;
	} else if (s.isEndElement()) {
	  break;
	} else {
	  qDebug() << "Unexpected entity in clipboard" << s.tokenType();
	}
      }
    } else if (s.isEndElement()) {
      break;
    } else {
      qDebug() << "Unexpected entity in clipboard at top" << s.tokenType();
    }
  }
  qDebug() << "Retrieved" << lst.size() << "objects";
  return lst;
}

  
