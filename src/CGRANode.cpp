#include "CGRANode.h"

CGRANode::CGRANode(int t_id, int t_x, int t_y) {
  m_id = t_id;
  m_disabled = false;
	m_hasDataMem = true;
  m_x = t_x;
  m_y = t_y;
	if(m_hasDataMem){
		m_supportOpts.insert("load");
		m_supportOpts.insert("store");
	}
}

void CGRANode::attachInLink(CGRALink* t_link) {
  m_inLinks.push_back(t_link);
}

void CGRANode::attachOutLink(CGRALink* t_link) {
  m_outLinks.push_back(t_link);
}

bool CGRANode::canSupport(string t_optsname){
	return m_supportOpts.find(t_optsname)!=m_supportOpts.end();
}
