#include "MRRG.h"

MRRG::MRRG(CGRA*t_cgra, int t_cycles){
	m_cgra = t_cgra;
	m_cycles = t_cycles;
	//Apply space and init data for LinkInfos
	for(int i=0;i<m_cgra->getLinkCount();i++){
		m_LinkInfos[m_cgra->links[i]] = new LinkInfo;
		m_LinkInfos[m_cgra->links[i]]->m_occupied_state = new int[m_cycles];
		for(int c=0;c<m_cycles;c++){
			m_LinkInfos[m_cgra->links[i]]->m_occupied_state[c] = LINK_NOT_OCCUPY;
		}
	}	
  for (int i=0; i<m_cgra->getrows(); ++i) {
    for (int j=0; j<m_cgra->getcolumns(); ++j) {
    	m_NodeInfos[m_cgra->nodes[i][j]]=new NodeInfo;
			m_NodeInfos[m_cgra->nodes[i][j]]->m_OccupiedByNode = new DFGNodeInst*[m_cycles];
			m_NodeInfos[m_cgra->nodes[i][j]]->m_Src1OccupyState = new int[m_cycles];
			m_NodeInfos[m_cgra->nodes[i][j]]->m_Src2OccupyState = new int[m_cycles];
			for(int c=0;c<m_cycles;c++){
				m_NodeInfos[m_cgra->nodes[i][j]]->m_OccupiedByNode[c] = NULL;
				m_NodeInfos[m_cgra->nodes[i][j]]->m_Src1OccupyState[c] = SRC_NOT_OCCUPY;
				m_NodeInfos[m_cgra->nodes[i][j]]->m_Src2OccupyState[c] = SRC_NOT_OCCUPY;
			}
    }
  }
}

MRRG::~MRRG(){
	for(int i=0;i<m_cgra->getLinkCount();i++){
		delete[] m_LinkInfos[m_cgra->links[i]]->m_occupied_state;
		delete m_LinkInfos[m_cgra->links[i]];
	}	
  for (int i=0; i<m_cgra->getrows(); ++i) {
    for (int j=0; j<m_cgra->getcolumns(); ++j) {
			delete[] m_NodeInfos[m_cgra->nodes[i][j]]->m_OccupiedByNode;
			delete[] m_NodeInfos[m_cgra->nodes[i][j]]->m_Src1OccupyState;
			delete[] m_NodeInfos[m_cgra->nodes[i][j]]->m_Src2OccupyState;
    	delete m_NodeInfos[m_cgra->nodes[i][j]];
    }
  }
}

void MRRG::MRRGclear(){
	for(int i=0;i<m_cgra->getLinkCount();i++){
		for(int c=0;c<m_cycles;c++){
			m_LinkInfos[m_cgra->links[i]]->m_occupied_state[c] = LINK_NOT_OCCUPY;
		}
	}	
  for (int i=0; i<m_cgra->getrows(); ++i) {
    for (int j=0; j<m_cgra->getcolumns(); ++j) {
			for(int c=0;c<m_cycles;c++){
				m_NodeInfos[m_cgra->nodes[i][j]]->m_OccupiedByNode[c] = NULL;
				m_NodeInfos[m_cgra->nodes[i][j]]->m_Src1OccupyState[c] = SRC_NOT_OCCUPY;
				m_NodeInfos[m_cgra->nodes[i][j]]->m_Src2OccupyState[c] = SRC_NOT_OCCUPY;
			}
    }
  }
}
