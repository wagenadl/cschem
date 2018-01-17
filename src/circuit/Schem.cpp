// Schem.cpp

#include "Schem.h"
#include "svg/SymbolLibrary.h"
#include <QDebug>

class SchemData: public QSharedData {
public:
  SchemData(bool valid): valid(valid) { }
public:
  Circuit circuit;
  //Symbols symbols;
  SymbolLibrary library;
  bool valid;
};  

Schem::Schem(bool valid) {
  d = new SchemData(valid);
}

Schem::Schem(Schem const &o) {
  d = o.d;
}

Schem::Schem(QXmlStreamReader &src): Schem() {
  d->valid = false;
  while (!src.atEnd()) {
    src.readNext();
    if (src.isStartElement()) {
      if (src.name()=="circuit") {
        src >> d->circuit;
	d->valid = d->circuit.isValid();
      //} else if (src.name()=="symbols") {
      //  src >> d->symbols;
      } else if (src.name()=="svg") {
        d->library.merge(src);
      } else {
        qDebug() << "Unexpected element in cschem: " << src.name();
	d->valid = false;
      }
    } else if (src.isEndElement()) {
      break;
    } else if (src.isCharacters() && src.isWhitespace()) {
    } else if (src.isComment()) {
    } else {
      qDebug() << "Unexpected entity in cschem: " << src.tokenType();
      d->valid = false;
    }
  }
  // now at end of schem element
}

Schem &Schem::operator=(Schem const &o) {
  d = o.d;
  return *this;
}

Schem::~Schem() {
}

Circuit const &Schem::circuit() const {
  return d->circuit;
}

void Schem::setCircuit(Circuit const &c) {
  d.detach();
  d->circuit = c;
}

//Symbols const &Schem::symbols() const {
//  return d->symbols;
//}
//
//void Schem::setSymbols(Symbols const &p) {
//  d.detach();
//  d->symbols = p;
//}

QXmlStreamReader &operator>>(QXmlStreamReader &sr, Schem &c) {
  c = Schem(sr);
  return sr;
}
  
QXmlStreamWriter &operator<<(QXmlStreamWriter &sr, Schem const &c) {
  sr.writeStartElement("cschem");
  sr.writeDefaultNamespace("http://www.danielwagenaar.net/cschem-ns.html");
  sr << c.circuit();
  //sr << c.symbols();
  c.saveSvg(sr);
  sr.writeEndElement();
  return sr;
};

SymbolLibrary const &Schem::library() const {
  return d->library;
}

void Schem::selectivelyUpdateLibrary(SymbolLibrary const &lib) {
  QSet<QString> syms;
  for (Element const &elt: circuit().elements())
    syms << elt.symbol();
  for (QString sym: syms) 
    if (lib.contains(sym))
      d->library.insert(lib.symbol(sym));
}

void Schem::saveSvg(QXmlStreamWriter &sw) const {
  sw.writeStartElement("svg");
  Symbol::writeNamespaces(sw);

  sw.setAutoFormatting(false);

  QSet<QString> syms;
  for (Element const &elt: circuit().elements())
    syms << elt.symbol();
  for (QString sym: syms) 
    if (d->library.contains(sym))
      d->library.symbol(sym).element().write(sw);
  
  sw.writeEndElement();

  sw.setAutoFormatting(true);

}

bool Schem::isEmpty() const {
  return circuit().isEmpty(); // && symbols().isEmpty();
}

bool Schem::isValid() const {
  return d->valid;
}
