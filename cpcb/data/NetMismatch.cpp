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
}

void NetMismatch::recalculate(PCBNet const &net, LinkedNet const &linkednet,
			      Group const &root) {
  /* Any of net.nodes that are not in linkednet.nodes should be colored PINK.
     Any nodes in linkednet.nodes that are not in net.nodes should be colored
     BLUE. We need to find them first. If they are not to be found, a MESSAGE
     should be shown in status bar.     
  */
  reset();
  
  QSet<Nodename> pcbnames;
  QSet<NodeID> maybewrong;
  for (NodeID const &pcbnode: net.nodes()) {
    Nodename name = root.nodeName(pcbnode);
    if (name.isValid() && name.pin()!="") {
      pcbnames << name;
      if (!linkednet.containsMatch(name)) {
        maybewrong << pcbnode;
      }
    }
  }
  maybewrong.remove(net.seed());
  if (pcbnames.size()==1) {
    NodeID id = root.findNodeByName(*pcbnames.begin());
    maybewrong.remove(id);
    // a net with one named node is not bad. By definition.
  }
  wronglyInNet |= maybewrong;

  // qDebug() << "NetMismatch::recalculate" << net.seed() << " : " << pcbnames;

  for (Nodename const &name: linkednet.nodes) {
    if (pcbnames.contains(name))
      continue;
    bool got = false;
    for (Nodename const &n: pcbnames) {
      if (n.matches(name)) {
	got = true;
	break;
      }
    }
    if (!got) {
      NodeID id = root.findNodeByName(name);
      // qDebug() << "looking for" << name << "gave" << id;
      if (!id.isEmpty())
	missingFromNet << id;
      else
	missingEntirely << name;
    }
  }

  //  if (!missingEntirely.isEmpty())
  //    wronglyInNet << net.seed(); // trick to make it colored
    
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
  qDebug() << "  wrongly in" << wronglyin.join(", ");
  qDebug() << "  wrongly out" << wronglyout.join(", ");
  qDebug() << "  missing" << missing.join(", ");
}

void NetMismatch::recalculateAll(LinkedSchematic const &ls,
				 Group const &root) {
  // Collect all PCB nets
  QMap<NodeID, PCBNet> seed2net;
  QSet<NodeID> allids;
  for (NodeID id: root.allPins()) {
    if (!allids.contains(id)) {
      PCBNet net = PCBNet(root, id);
      allids |= net.nodes();
      seed2net[id] = net;
    }
  }

  // For each linked schematic net, find corresponding pcb net
  QSet<NodeID> donenets;
  for (LinkedNet const &lnet: ls.nets()) {
    bool got = false;
    for (NodeID const &node: seed2net.keys()) {
      Nodename seed = seed2net[node].someNode();
      if (lnet.containsMatch(seed)) {
	// schematic net "lnet" matches pcb net for "seed"
	if (donenets.contains(node)) {
	  // we've already studied this pcb net!?
	  qDebug() << "Double match";
	} else {
	  NetMismatch nm1;
	  nm1.recalculate(seed2net[node], lnet, root);
	  wronglyInNet |= nm1.wronglyInNet;
	  missingFromNet |= nm1.missingFromNet;
	  missingEntirely |= nm1.missingEntirely;
	  donenets << node;
	  got = true;
	  if (!nm1.wronglyInNet.isEmpty()
	      || !nm1.missingFromNet.isEmpty()
	      || !nm1.missingEntirely.isEmpty()) {
	    qDebug() << "linked net" << lnet;
            qDebug() << "  node" << node ;
            qDebug() << "  seed" << seed ;
            qDebug() << "=== pcbnet report:";
            seed2net[node].report();
            qDebug() << "=== netmismatch report:";
	    nm1.report(root);
	  }
	}
      }
    }
    if (!got) {
      qDebug() << "No match for " << lnet;
    }
  }
  qDebug() << "prefinal" << wronglyInNet.size();
  for (NodeID const &node: seed2net.keys()) {
    if (!donenets.contains(node)) {
      qDebug() << "checking" << root.nodeName(node);
      QSet<NodeID> const &nodes = seed2net[node].nodes();
      bool havename = false;
      for (NodeID const &n: nodes) {
        Nodename name = root.nodeName(n);
        qDebug() << "  considering" << name;
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

  
