// NetMismatch.cpp

#include "NetMismatch.h"
#include "PCBNet.h"
#include "LinkedNet.h"
#include "Group.h"
#include "PinMapper.h"
#include  "LinkedSchematic.h"

NetMismatch::NetMismatch() {
}

void NetMismatch::reset() {
  wronglyInNet.clear();
  missingFromNet.clear();
  missingEntirely.clear();
  incompleteNets.clear();
  overcompleteNets.clear();
}

void NetMismatch::recalculate(PCBNet const &net, LinkedNet const &linkednet,
			      Group const &root) {
  /* Any of net.nodes that are not in linkednet.nodes should be colored PINK.
     Any nodes in linkednet.nodes that are not in net.nodes should be colored
     BLUE. We need to find them first. If they are not to be found, a MESSAGE
     should be shown in status bar.     
  */
  reset();
  
  QMap<NodeID, Nodename> pcbnames;
  for (NodeID const &pcbnode: net.nodes()) {
    Nodename name = root.nodeName(pcbnode);
    if (name.isValid() && name.pin()!="") {
      pcbnames[pcbnode] = name;
      if (!linkednet.containsMatch(name)) {
        wronglyInNet << pcbnode;
      }
    }
  }
  wronglyInNet.remove(net.seed());
  if (pcbnames.size()==1) {
    for (auto id: root.findNodesByName((pcbnames.begin().value())))
      wronglyInNet.remove(id);
    // a net with one named node is not bad. By definition.
  }

  // qDebug() << "NetMismatch::recalculate" << net.seed() << " : " << pcbnames;

  for (Nodename const &name: linkednet.nodes) {
    QSet<NodeID> contained;
    for (auto it=pcbnames.begin(); it!=pcbnames.end(); it++) {
      if (it.value().matches(name))
        contained << it.key();
    }
    QList<NodeID> ids = root.findNodesByName(name);
    qDebug() << "looking for" << name << "gave" << ids;
    if (ids.isEmpty())
      missingEntirely << name;
    else
      for (auto id: ids)
        if (!contained.contains(id))
          missingFromNet << id;
    }

  //  if (!missingEntirely.isEmpty())
  //    wronglyInNet << net.seed(); // trick to make it colored

  if (!missingFromNet.isEmpty())
    incompleteNets << linkednet.name;
  if (!wronglyInNet.isEmpty())
    overcompleteNets << linkednet.name;
}

void NetMismatch::report(Group const &root) {
  qDebug() << "NetMismatch";
  QStringList wronglyin;
  for (NodeID const &id: wronglyInNet) 
    wronglyin << root.nodeName(id).toString();
  QStringList wronglyout;
  for (NodeID const &id: missingFromNet) 
    wronglyout << root.nodeName(id).toString();
  QStringList missing;
  for (Nodename const &n: missingEntirely)
    missing << n.toString();
  QStringList incomplete;
  for (QString net: incompleteNets)
    incomplete << net;
  QStringList overcomplete;
  for (QString net: overcompleteNets)
    overcomplete << net;
  qDebug() << "  wrongly in" << wronglyin.join(", ");
  qDebug() << "  wrongly out" << wronglyout.join(", ");
  qDebug() << "  missing" << missing.join(", ");
  qDebug() << "  incomplete" << incomplete.join(", ");
  qDebug() << "  overcomplete" << overcomplete.join(", ");
}

void NetMismatch::recalculateAll(LinkedSchematic const &ls,
				 Group const &root) {
  reset();

  // Collect all PCB nets
  QMap<NodeID, PCBNet> seed2net;
  QSet<NodeID> allids;
  for (NodeID id: root.allPins()) {
    if (allids.contains(id))
      continue;
    Nodename n = root.nodeName(id);
    if (n.pin()=="")
      continue;
    // only use pins with a name or number as seed
    PCBNet net = PCBNet(root, id);
    allids |= net.nodes();
    seed2net[id] = net;
  }
  qDebug() << "Collected" << allids;

  // For each linked schematic net, find corresponding pcb net
  QSet<NodeID> donenets;
  for (LinkedNet const &lnet: ls.nets()) {
    bool got = false;
    for (NodeID const &node: seed2net.keys()) {
      Nodename seed = seed2net[node].someNode();
      if (!lnet.containsMatch(seed))
        continue;
      // schematic net "lnet" matches pcb net for "seed"
      if (donenets.contains(node)) {
        // we've already studied this pcb net!?
        qDebug() << "Double match";
        continue;
      }
      if (got) {
        qDebug() << "lnet already taken";
        missingFromNet |= seed2net[node].nodes();
        incompleteNets << lnet.name;
        donenets << node;
      } else {
        NetMismatch nm1;
        nm1.recalculate(seed2net[node], lnet, root);
        wronglyInNet |= nm1.wronglyInNet;
        missingFromNet |= nm1.missingFromNet;
        missingEntirely |= nm1.missingEntirely;
        incompleteNets |= nm1.incompleteNets;
        overcompleteNets |= nm1.overcompleteNets;
        donenets << node;
        got = true;
        if (!nm1.wronglyInNet.isEmpty()
            || !nm1.missingFromNet.isEmpty()
            || !nm1.missingEntirely.isEmpty()) {
          qDebug() << "problematic linked net" << lnet;
          qDebug() << "  node" << node ;
          qDebug() << "  seed" << seed ;
          qDebug() << "=== pcbnet report:";
          seed2net[node].report();
          qDebug() << "=== netmismatch report:";
          nm1.report(root);
        }
      }
    }
    if (!got) {
      qDebug() << "No match for " << lnet;
    }
  }
  for (NodeID const &node: seed2net.keys()) {
    if (!donenets.contains(node)) {
      QSet<NodeID> const &nodes = seed2net[node].nodes();
      bool havename = false;
      for (NodeID const &n: nodes) {
        Nodename name = root.nodeName(n);
        if (name.pin()!="") {
          if (havename) {
            wronglyInNet |= nodes;
            break;
          } else {
            havename = true;
          }
        }
      }
    }
  }
}

  
