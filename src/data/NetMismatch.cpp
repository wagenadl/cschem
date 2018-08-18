// NetMismatch.cpp

#include "NetMismatch.h"
#include "PCBNet.h"
#include "LinkedNet.h"
#include "Group.h"
#include "PinMapper.h"

NetMismatch::NetMismatch() {
}

void NetMismatch::reset() {
  wronglyInNet.clear();
  missingFromNet.clear();
  missingEntirely.clear();
}

void NetMismatch::recalculate(PCBNet const &net, LinkedNet const &linkednet,
			      Group const &root) {
  /* Any of net.nodes that are not in linkednet.nodes should be colored ORANGE.
     Any nodes in linkednet.nodes that are not in net.nodes should be colored
     BLUE. We need to find them first. If they are not to be found, a MESSAGE
     should be shown in status bar.     
  */
  reset();
  
  QSet<Nodename> pcbnames;
  for (NodeID const &pcbnode: net.nodes()) {
    Nodename name = root.nodeName(pcbnode);
    if (name.isValid()) {
      pcbnames << name;
      if (!linkednet.containsMatch(name))
	wronglyInNet << pcbnode;
    }
  }
  wronglyInNet.remove(net.seed());

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
      if (!id.isEmpty())
	missingFromNet << id;
      else
	missingEntirely << name;
    }
  }
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
