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

/** this function is used to get all possible map paths for a DFGNode.
 *	it get paths to every CGRANodes, and save pathes in t_paths
 *	@param t_InstNode: the DFGNode need to be mapped
 *	@param t_paths: save the pathes
 */
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

/** this function try to get a map path to a certain CGRANode for a DFGNode .
 *  if t_InstNode has predNodes have been mapped,it will travels all mapped preNodes and try to find a shortest path from the CGRANode which a preNode is mapped,to the CGRANode which t_InstNode want to map to.
 *	@param t_InstNode: the DFGNode need to be mapped
 *	@param t_cgraNode: the CGRANode where the DFGNode is hoped to map to
 *	@return the path
 */
map<CGRANode*,int>* Mapper::getPathforInstNodetoCGRANode(DFGNodeInst* t_InstNode,CGRANode* t_cgraNode){
#ifdef CONFIG_MAP_DEBUG 
		outs()<<"Try to find path for DFGNode"<<t_InstNode->getID()<<" to "<<"CGRANode" << t_cgraNode->getID()<<"\n"; 
#endif
			map<CGRANode*,int>* path = NULL;
			//bool allPredNodenotMapped = true;
			for(DFGNodeInst* preInstNode: *(t_InstNode->getPredInstNodes())){
				if(m_mapInfo[preInstNode]->mapped == true){
					if(t_cgraNode->canSupport(t_InstNode->getOpcodeName())){
						//create a tmp path,to save the path from the pre cgra node to the t_cgra node
						//we only need to save the shortest path,delete other temppaths
						map<CGRANode*,int>* temppath = new map<CGRANode*,int>;
						Dijkstra_search(temppath,preInstNode,t_InstNode,m_mapInfo[preInstNode]->cgraNode,t_cgraNode);
					}else{
						;
					}
				//	allPredNodenotMapped = false;
				}
			}
			return path;
}

/** this function try to find a path to a certain CGRANode for a DFGNode,when this DFGNode has some predNode which has been mapped.The path is from the CGRANode which map the preDFGNode to the CGRANode where the succDFGNode want to map
 * use dijkstara search to find this path,if find this path, save it in t_path
 */
void Mapper::Dijkstra_search(map<CGRANode*,int>* t_path,DFGNodeInst* t_srcDFGNode,DFGNodeInst* t_dstDFGNode,CGRANode* t_srcCGRANode,CGRANode* t_dstCGRANode){
	list<CGRANode*> searchPool;
	map<CGRANode*,int> distance;//used to save the distance from t_srcCGRANode to a CGRANode on path
	map<CGRANode*,int> timing;//used to save the clock cycle when arrive a CGRANode on path
	map<CGRANode*,CGRANode*> previous;//used to save the predCGRANode on Path
	bool successFindPath = false;//record if successd find the path
	
	//init data
	for(int r=0;r<m_cgra->getrows();r++){
		for(int c = 0;c<m_cgra->getcolumns();c++){
			CGRANode* node = m_cgra->nodes[r][c];
			distance[node] = m_mrrg->getMRRGcycles();//init the distance to the max value
			timing[node] = m_mapInfo[t_srcDFGNode]->cycle;
			previous[node] = NULL;
			searchPool.push_back(node);
		}
	}
	distance[t_srcCGRANode] = 0;//set the distance of start node 0,start search

	while (searchPool.size()!=0){//search the searchPool,until find the path to t_dstCGRANode
		int mindistance = m_mrrg->getMRRGcycles() + 1;
		CGRANode* mindisCGRANode;
		//find the mindistance node,this also choose a path,and this node is the latest node in path, we will try to search the next node and add to this path.
		for(CGRANode* node:searchPool){
			if(distance[node] < mindistance){
				mindistance = distance[node];
				mindisCGRANode = node;
			}
		}
		searchPool.remove(mindisCGRANode);

		if(mindisCGRANode == t_dstCGRANode){//find the target CGRANode,exit searching
			timing[t_dstCGRANode] = //!!!!!!TODO
			successFindPath  = true;
			break;
		}

		//haven't find the target CGRANode,update the distance and arrive timing from t_srcCGRANode to currentpath's latest node's neighbor nodes on current path.(add path head's neighbor to the path)
		list<CGRANode*>* NeighborCGRANodes = mindisCGRANode->getNeighbors();
		for(CGRANode* neighbor: *NeighborCGRANodes){
			int cycle = timing[mindisCGRANode];
			while(1){
				CGRALink* currentLink = mindisCGRANode->getOutLinkto(neighbor);
				//if the CGRALink to neighbor can be occupied in this cycle in MRRG, then add the neighbor to current path.
				if(m_mrrg->canOccupyLink(currentLink,cycle,m_II)){
					int cost = distance[mindisCGRANode]+(cycle - timing[mindisCGRANode]) + 1;
					if(cost < distance[neighbor]){
						distance[neighbor] = cost;
						timing[neighbor] = cycle + 1;
						previous[neighbor] = mindisCGRANode;
					}
					break;
				}
				//this cycle the CGRALink is occupied, add cycle to try again.
				cycle ++;
				//if the cycle > maxcycle of MRRG,main we can't add this neighbor to current path give up this neighbor start trying next neighbor,if all neighbor can't been add to the path is also ok,distance,timing,and previous is not changed,and this path's head have been delete in search pool, next search will choose another path.
				if(cycle > m_mrrg->getMRRGcycles())
					break;
			}
		}
	}
}

void Mapper::mapInfoInit(){
	for(DFGNodeInst* InstNode: *(m_dfg->getInstNodes())){
		m_mapInfo[InstNode]->cgraNode = NULL;
		m_mapInfo[InstNode]->cycle = 0;
		m_mapInfo[InstNode]->mapped = true;
	}
}

Mapper::~Mapper(){
	for(DFGNodeInst* InstNode: *(m_dfg->getInstNodes())){
		delete m_mapInfo[InstNode];
	}
}
