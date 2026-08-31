// NetExporter.cpp

#include "NetExporter.h"

#include <QFile>
#include <QString>
#include <QRegularExpression>

#include "svg/Symbol.h"
#include "svg/SymbolLibrary.h"
#include "circuit/Net.h"
#include "circuit/Element.h"
#include <svg/Geometry.h>

QString clean(QString s) {
  s.replace(".", "_");
  s.replace("−", "-");
  s.replace("/", "_");
  s.replace(":", "-");
  return s.replace(QRegularExpression("[^-+_.,0-9A-Za-z]"), "");
}

QString refname(Element const &elt) {
  if (elt.name.isEmpty()) 
    return "elt-" + QString::number(elt.id);
  return clean(elt.name);
}


QString symbolname(QString name) {
  name.replace(QRegularExpression("^part:"), "");
  return clean(name);
}

QString symbolname(Element const &elt) {
  return symbolname(elt.symbol());
}

QString symbolname(Symbol const &sym) {
  return symbolname(sym.typeName());
}

QString pinname(QString pin) {
  return clean(pin);
}

QString netname(Net const &net) {
  return clean(net.name());
}

void writeNet(Net const &net, Circuit const &circ, QTextStream &out) {
  out << "  (net " << netname(net);
  for (PinID const &pin: net.pins()) {
    if (pin.element() <= 0)
      continue;
    Element const &elt(circ.elements[pin.element()]);
    if (elt.type != Element::Type::Component)
      continue;
    out << " (" << refname(elt);
    if (!pin.pin().isEmpty())
      out << " " << pinname(pin.pin()) << ")";
    else
      out << ")";    
  }
  out << ")\n";
}

void writeElement(Element const &elt, QString pfx,
                   QTextStream &out, QSet<int> &seen) {
  out << pfx << " " <<refname(elt) << " " << symbolname(elt)
      << " " << elt.position.x() << " " << -elt.position.y()
      << " " << QString::number((360 - 90*int(elt.rotation)) % 360)
      << " " << (elt.flipped ? 1 : 0) << ")\n";
  seen << elt.id;
}

void writeCompound(Circuit const &circ, int containerid,
                   QTextStream &out, QSet<int> &seen) {
  out << "  (compound\n";
  writeElement(circ.elements[containerid], "    (container", out, seen);
  for (int id: circ.containedElements(containerid))
    writeElement(circ.elements[id], "    (sub", out, seen);
  out << "  )\n";
}

void writeSymbol(Symbol const &sym, QTextStream &out) {
  out << "  (symbol " << symbolname(sym) << "\n";
  for (QString const &pin: sym.pinNames()) {
    QPointF p(sym.shiftedPinPosition(pin));
    out << "    (pin " << pinname(pin) <<
      " " << QString::number(int(p.x()/7))
        << " " << QString::number(int(-p.y()/7)) << ")\n";
  }
  out << "  )\n";    
}

void writeConnection(Connection const &con, Geometry const &geom, QTextStream &out) {
  if (!con.isValid())
    return;
  if (con.isDangling())
    return;
  QList<QPoint> points;
  points << geom.pinPosition(con.from());
  for (QPoint const &p: con.via)
    points << p;
  points << geom.pinPosition(con.to());
  for (int k=0; k<points.size() - 1; k++) {
    out << "  (wire " << QString::number(points[k].x())
        << " " << QString::number(-points[k].y())
        << " " << QString::number(points[k+1].x())
        << " " << QString::number(-points[k+1].y()) << ")\n";
  }
}

QRegularExpression bits("^([^0-9]*)([0-9.]*)(.*)");

bool naturalOrder(QString a, QString b) {
  auto mtcha(bits.match(a));
  auto mtchb(bits.match(b));
  if (mtcha.captured(1) == mtchb.captured(1)) {
    if (mtcha.captured(2) == mtchb.captured(2)) {
      return mtcha.captured(3) < mtchb.captured(3);
    } else {
      return mtcha.captured(2).toFloat() < mtchb.captured(2).toFloat();
    }
  } else {
    return mtcha.captured(1) < mtchb.captured(1);
  }
}

bool NetExporter::exportNet(QString ofn) const {
  Circuit const &circ(s.circuit());

  QFile file(ofn);
  if (ofn == "-") {
    file.open(stdout, QFile::WriteOnly);
  } else {
    if (!file.open(QFile::WriteOnly)) {
      qDebug() << "Failed to save" << ofn;
      return false;    
    }
  }
  QTextStream out(&file);
  out << "(cschem\n";

  // components and compounds
  QSet<int> seen;
  QList<int> elts = circ.elements.keys();
  std::sort(elts.begin(), elts.end(), [&circ](int a, int b) {
    return naturalOrder(circ.elements[a].name, circ.elements[b].name); });
  for (int id: elts) {
    Element const &elt(circ.elements[id]);
    if (elt.type != Element::Type::Component)
      continue;
    if (seen.contains(id))
      continue; // skip container/contained
    int cid = circ.containerOf(elt.id);
    if (cid > 0) 
      writeCompound(circ, cid, out, seen);
    else if (!circ.containedElements(elt.id).isEmpty()) 
      writeCompound(circ, elt.id, out, seen);
    else 
      writeElement(elt, "  (component", out, seen);
  }

  // nets
  QMap<QString, Net> nets;
  QMap<QString, QString> portrevmap;
  for (Net const &net: Net::allNets(circ)) {
    QString name = net.name();
    for (QString const &p: net.ports())
      if (p != name)
        portrevmap[p] =name;
  }
  
  for (Net const &net: Net::allNets(circ)) {
    QString name = net.name();
    if (portrevmap.contains(name))
      name = portrevmap[name];
    if (nets.contains(name))
      nets[name].merge(net);
    else
      nets[name] = net;
  }
  for (Net const &net: nets)
    writeNet(net, circ, out);

  // wires
  SymbolLibrary const &lib(s.library());
  Geometry geom(circ, lib);
  for (Connection const &con: circ.connections) 
    writeConnection(con, geom, out);

  // symbols
  QMap<QString, int> symbols;
  for (int id: seen)
    symbols[circ.elements[id].symbol()]++;
  for (QString const &name: symbols.keys()) {
    Symbol const &sym(s.library().symbol(name));
    writeSymbol(sym, out);
  }
  out << ")\n";

  return true;
}
