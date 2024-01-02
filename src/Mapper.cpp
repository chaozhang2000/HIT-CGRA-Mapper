#include "Mapper.h"
#include <cmath>
#include "common.h"

using namespace std;
Mapper::Mapper(DFG* t_dfg,CGRA* t_cgra,MRRG* t_mrrg){
	m_cgra = t_cgra;
	m_dfg = t_dfg;
	m_mrrg = t_mrrg;
	m_II = getResMII();
	for(DFGNodeInst* InstNode: *(m_dfg->getInstNodes())){
		m_mapInfo[InstNode] = new MapInfo;
	}

}

int Mapper::getResMII(){
	int ResMII=ceil(float(m_dfg->getInstNodeCount())/m_cgra->getNodeCount());
	return ResMII;
}

void Mapper::heuristicMap(){
#ifdef CONFIG_MAP_DEBUG 
	OUTS("\nMAP DEBUG",ANSI_FG_BLUE); 
#endif
	while(1){
#ifdef CONFIG_MAP_DEBUG 
		OUTS("==================================",ANSI_FG_CYAN); 
		OUTS("start heuristic algorithm with II="<<m_II,ANSI_FG_CYAN); 
#endif
		m_mrrg->MRRGclear();
		mapInfoInit();

		//Traverse all InstNodes in dfg, try to map them one by one
		for(list<DFGNodeInst*>::iterator InstNode=m_dfg->getInstNodes()->begin();InstNode!=m_dfg->getInstNodes()->end(); InstNode ++){
			list<map<CGRANode*,int>*> paths;
			getMapPathsforInstNode(*InstNode,&paths);
			//paths has no more use,delete
			for(map<CGRANode*,int>* path : paths){delete path;}
		}
		m_II ++;
	}
}

void Mapper::getMapPathsforInstNode(DFGNodeInst* t_InstNode,list<map<CGRANode*,int>*>* t_paths){
#ifdef CONFIG_MAP_DEBUG 
		outs()<<"Try to find paths for DFGNode"<<t_InstNode->getID()<<" to each CGRANode\n"; 
#endif
	for(int r = 0; r< m_cgra->getrows();r++){
		for(int c = 0; c< m_cgra->getcolumns();c++){
			CGRANode* cgraNode = m_cgra->nodes[r][c];
			map<CGRANode*,int>* path = getPathforInstNodetoCGRANode(t_InstNode,cgraNode);
			t_paths->push_back(path);
		}
	}
}

map<CGRANode*,int>* Mapper::getPathforInstNodetoCGRANode(DFGNodeInst* t_InstNode,CGRANode* t_cgraNode){
#ifdef CONFIG_MAP_DEBUG 
		outs()<<"Try to find path for DFGNode"<<t_InstNode->getID()<<" to "<<"CGRANode" << t_cgraNode->getID()<<"\n"; 
#endif
			//create a path
			map<CGRANode*,int>* path = new map<CGRANode*,int>;
			//bool allPredNodenotMapped = true;
			for(DFGNodeInst* preInstNode: *(t_InstNode->getPredInstNodes())){
				if(m_mapInfo[preInstNode]->mapped == true){
					if(t_cgraNode->canSupport(t_InstNode->getOpcodeName())){
						outs()<<t_InstNode->getOpcodeName()<<"is supported\n";
					}
				//	allPredNodenotMapped = false;
				}
			}
			return path;
}

/*void Mapper::Dijkstra_search(map<CGRANode*,int>* path,){
	;
}*/

void Mapper::mapInfoInit(){
	for(DFGNodeInst* InstNode: *(m_dfg->getInstNodes())){
		m_mapInfo[InstNode]->cgra = NULL;
		m_mapInfo[InstNode]->cycle = 0;
		m_mapInfo[InstNode]->mapped = false;
	}
}

Mapper::~Mapper(){
	for(DFGNodeInst* InstNode: *(m_dfg->getInstNodes())){
		delete m_mapInfo[InstNode];
	}
}
