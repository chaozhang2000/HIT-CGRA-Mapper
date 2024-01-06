#include "MRRG.h"
#include <cassert>

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

bool MRRG::canOccupyLink(CGRALink* t_cgraLink,int t_cycle,int t_II){
	for(int c=t_cycle;c<m_cycles;c=c+t_II){
		if(m_LinkInfos[t_cgraLink]->m_occupied_state[c] != LINK_NOT_OCCUPY)
						return false;
	}
	return true;
}

bool MRRG::canOccupyNode(CGRANode* t_cgraNode,int t_cycle,int t_II){
	for(int c=t_cycle;c<m_cycles;c=c+t_II){
		if(m_NodeInfos[t_cgraNode]->m_OccupiedByNode[c] != NULL)
						return false;
	}
	return true;
}

/**TODO: m_Src1OccupyState and m_Src2OccupyState not be handled yet
 */
void MRRG::scheduleNode(CGRANode* t_cgraNode,DFGNodeInst* t_dfgNode,int t_cycle,int t_II){
	for(int c=t_cycle;c<m_cycles;c=c+t_II){
		if(m_NodeInfos[t_cgraNode]->m_OccupiedByNode[c] != NULL){
			assert("The CGRANode in path can't be schedule,this should not happen, the Mapper has some bugs");
		}else{
			m_NodeInfos[t_cgraNode]->m_OccupiedByNode[c] = t_dfgNode;
		}
	}
}

void MRRG::scheduleLink(CGRALink* t_cgraLink,int t_cycle,int duration,int t_II){
	for(int c=t_cycle;c<m_cycles;c=c+t_II){
		for(int d = 0;d<duration and c+d < m_cycles;d++){
			if(m_LinkInfos[t_cgraLink]->m_occupied_state[c+d]!=LINK_NOT_OCCUPY){
				assert("The CGRALink in path can't be schedule,this should not happen, the Mapper has some bugs");
			}
			else{
				m_LinkInfos[t_cgraLink]->m_occupied_state[c+d]=LINK_OCCUPY_FROM_N;
			}
		}
	}
}
