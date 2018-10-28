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

QMimeData *Clipboard::createMimeData(Group const &g) {
  QBuffer buf;
  buf.open(QBuffer::WriteOnly);
  { QXmlStreamWriter s(&buf);
    s.writeStartDocument("1.0", false);
    s.writeStartElement("cpcbsel");
    s.writeDefaultNamespace("http://www.danielwagenaar.net/cpcbsel-ns.html");
    s << g;
    s.writeEndElement();
    s.writeEndDocument();
  }
  buf.close();
  QMimeData *md = new QMimeData;
  md->setData(dndformat, buf.buffer());
  return md;
}

void Clipboard::store(Group const &root, QSet<int> selection) {
  Group g(root.subset(selection));
  QMimeData *md = createMimeData(g);
  QClipboard *cb = QApplication::clipboard();
  cb->setMimeData(md);
}

Group Clipboard::parseMimeData(QMimeData const *md) {
  QByteArray ba(md->data(dndformat));
  QBuffer buf(&ba);
  qDebug() << "Retrieving" << QString(buf.buffer());
  if (!buf.open(QBuffer::ReadOnly)) {
    qDebug() << "Failed to open clipboard buffer";
    return Group();
  }
  QXmlStreamReader s(&buf);
  while (!s.atEnd()) {
    s.readNext();
    if (s.isStartElement() && s.name()=="cpcbsel") {
      while (!s.atEnd()) {
        s.readNext();
        if (s.isStartElement()) {
          qDebug() << "in cpcbsel" << s.name();
          Object o;
          s >> o;
          qDebug() << "Retrieved" << o;
          if (o.isGroup())
            return o.asGroup();
          else
            return Group(); // this is an error
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
  return Group();
}

Group Clipboard::retrieve() const {
  if (!isValid())
    return Group();
  QClipboard *cb = QApplication::clipboard();
  QMimeData const *md = cb->mimeData();
  return parseMimeData(md);
}

  
