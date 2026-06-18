// NetMismatch.cpp

#include "NetMismatch.h"
#include "LinkedNet.h"
#include "Group.h"

#include  "LinkedSchematic.h"
#include "NetGraph.h"

#include <QTime>

NetMismatch::NetMismatch() {
}

void NetMismatch::reset() {
  wronglyInNet.clear();
  missingFromNet.clear();
  missingEntirely.clear();
  incompleteNets.clear();
  overcompleteNets.clear();
}

void NetMismatch::recalculate(QSet<NodeID> const &net, const NodeID &seed,
                              LinkedNet const &linkednet,
			      Group const &root) {
  /* Any of net.nodes that are not in linkednet.nodes should be colored PINK.
     Any nodes in linkednet.nodes that are not in net.nodes should be colored
     BLUE. We need to find them first. If they are not to be found, a MESSAGE
     should be shown in status bar.     
  */
  reset();
  
  QMap<NodeID, Nodename> pcbnames;
  for (NodeID const &nid: net) {
    Nodename name = root.nodeName(nid);
    if (name.pin() != "") {
      pcbnames[nid] = name;
      if (!linkednet.containsMatch(name)) 
        wronglyInNet << nid;
    }
  }
  wronglyInNet.remove(seed);
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
    //    qDebug() << "looking for" << name << "gave" << ids;
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
  QList<QSet<NodeID>> nets = NetGraph(root).allNets();
  int K = nets.size();
  QList<NodeID> seeds(K); // in same order
  QList<Nodename> netnames(K); // ditto
  for (int k=0; k<K; k++) {
    for (NodeID const &nid: nets[k]) {
      Nodename n = root.nodeName(nid);
      if (n.pin() != "") {
        seeds[k] = nid;
        netnames[k] = n;
        break;
      }
    }
  }
  // Now we have names for our nets. Not necessarily for all, but unnamed nets
  // cannot have importance (?)

  // For each linked schematic net, find corresponding pcb net
  QSet<int> donenets; // into the "nets" list
  for (LinkedNet const &lnet: ls.nets()) {
    bool got = false;
    for (int k=0; k<K; k++) {
      if (!lnet.containsMatch(netnames[k]))
        continue;
      // schematic net "lnet" matches pcb net k
      if (donenets.contains(k)) {
        // we've already studied this pcb net!?
        qDebug() << "Double match";
        continue;
      }
      if (got) {
        qDebug() << "lnet already taken";
        missingFromNet |= nets[k];
        incompleteNets << lnet.name;
        donenets << k;
      } else {
        NetMismatch nm1;
        nm1.recalculate(nets[k], seeds[k], lnet, root);
        wronglyInNet |= nm1.wronglyInNet;
        missingFromNet |= nm1.missingFromNet;
        missingEntirely |= nm1.missingEntirely;
        incompleteNets |= nm1.incompleteNets;
        overcompleteNets |= nm1.overcompleteNets;
        donenets << k;
        got = true;
        if (!nm1.wronglyInNet.isEmpty()
            || !nm1.missingFromNet.isEmpty()
            || !nm1.missingEntirely.isEmpty()) {
          qDebug() << "problematic linked net" << lnet;
          qDebug() << "  node" << seeds[k];
        }
      }
    }
    if (!got) {
      qDebug() << "No match for " << lnet;
    }
  }
  
  // any uncaptured nets are superfluous. but if they comprise zero or one named pins, no one cares
  for (int k=0; k<K; k++) {
    if (!donenets.contains(k)) {
      if (netnames[k].pin() != "") {
        // if this net contains more than one named pin, that's a problem
        int count = 0;
        for (NodeID const &nid: nets[k]) {
          if (root.nodeName(nid).pin() != "") {
            count ++;
            if (count >= 2) {
              wronglyInNet |= nets[k];
              break;
            }
          }
        }
      }
    }
  }
}

  
