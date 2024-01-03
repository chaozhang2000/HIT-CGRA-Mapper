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
			if(cgraNode->canSupport(t_InstNode->getOpcodeName()) and cgraNode->isdisable()==false){
				map<CGRANode*,int>* path = getPathforInstNodetoCGRANode(t_InstNode,cgraNode);
				t_paths->push_back(path);
			}
		}
	}
}

/** this function try to get a map path to a certain CGRANode for a DFGNode .
 *  if t_InstNode has predNodes have been mapped,it will travels all mapped preNodes and try to find a longest path from the CGRANode which a preNode is mapped,to the CGRANode which t_InstNode want to map to.choose the longest path because we need to route all preNode to this node, if choose the shorest path,some preNode will route failed.
 *	@param t_InstNode: the DFGNode need to be mapped
 *	@param t_cgraNode: the CGRANode where the DFGNode is hoped to map to
 *	@return the path
 */
map<CGRANode*,int>* Mapper::getPathforInstNodetoCGRANode(DFGNodeInst* t_InstNode,CGRANode* t_cgraNode){
#ifdef CONFIG_MAP_DEBUG 
		outs()<<"Try to find path for DFGNode"<<t_InstNode->getID()<<" to "<<"CGRANode" << t_cgraNode->getID()<<"\n"; 
#endif
			map<CGRANode*,int>* path = NULL;
			bool allPredNodenotMapped = true;
			int maxpathlastcycle = 0;
			list<map<CGRANode*,int>*> temppaths;//save the temppath, used to delete no use temppaths.
			for(DFGNodeInst* preInstNode: *(t_InstNode->getPredInstNodes())){
				if(m_mapInfo[preInstNode]->mapped == true){
					allPredNodenotMapped = false;
					//create a tmp path,to save the path from the pre cgra node to the t_cgra node
					//we only need to save the shortest path,delete other temppaths
					map<CGRANode*,int>* temppath = Dijkstra_search(preInstNode,t_InstNode,m_mapInfo[preInstNode]->cgraNode,t_cgraNode);
					//after try to find a path,if the Dijkstra_search return NULL,mean the path to t_cgraNode not exsist,because not every mapped preNode has path to t_cgraNode
					if(temppath == NULL){
						if(temmppaths.size()!=0){
							for(map<CGRANode*,int>* deletepath: temppaths){
								delete deletepath;
							}
						}
						return NULL;
					//if the temmpath is found save it to the tmppaths,and find the longest temmpath and save to path.
					}else{
						temppaths.push_back(tmeppath);
						if((*temppath)[t_cgraNode]>= maxpathlastcycle){
							maxpathlastcycle = (*temppath)[t_cgraNode];
							path = tempPath
						}
					}
				}
			}
			//program reach here, has two situations,
			//1.the path has already found
			//2.allPredNodenotMapped == true,path not found
			//handle situation one
			if(path !=NULL){
				for(map<CGRANode*,int>* noUseTempPath: temppaths){
					if (noUseTempPath != path) delete noUseTempPath;
				}
				return path;
			}
			//handle situation two
			if(allPredNodenotMapped == true){
			
			}
			return NULL;

}

/** this function try to find a path to a certain CGRANode for a DFGNode,when this DFGNode has some predNode which has been mapped.The path is from the CGRANode which map the preDFGNode to the CGRANode where the succDFGNode want to map
 * use dijkstara search to find this path,if find this path,return path,else return NULL
 */
map<CGRANode*,int>* Mapper::Dijkstra_search(DFGNodeInst* t_srcDFGNode,DFGNodeInst* t_dstDFGNode,CGRANode* t_srcCGRANode,CGRANode* t_dstCGRANode){
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
			int time = timing[t_dstCGRANode];
			while(time < m_mrrg->getMRRGcycles()){
				if(m_mrrg->canOccupyNode(t_dstCGRANode,time,m_II)==true){
					successFindPath  = true;//search to the t_dstCGRANode and the t_dstCGRANode can be occupy,mean find path successfully.
					break;
				}
				time++;
			}
			timing[t_dstCGRANode] = time;
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
	//if the path is found, add it to path and return path
	map<CGRANode*,int>* path = NULL;
	if(successFindPath){
	 CGRANode* u = t_dstCGRANode;
	 map<CGRANode*,int> reverseOrderPath;
	 path =  new map<CGRANode*,int>;
	 while(u!=NULL){
	 	reverseOrderPath[u] = timing[u];
		u = previous[u];
	 }	 
	 for(map<CGRANode*,int>::reverse_iterator rit = reverseOrderPath.rbegin();rit != reverseOrderPath.rend();++rit) {
			(*path)[rit->first]=rit->second;
	 }
	}
	return path;
}

void Mapper::mapInfoInit(){
	for(DFGNodeInst* InstNode: *(m_dfg->getInstNodes())){
		m_mapInfo[InstNode]->cgraNode = NULL;
		m_mapInfo[InstNode]->cycle = 0;
		m_mapInfo[InstNode]->mapped = false;
	}
}

Mapper::~Mapper(){
	for(DFGNodeInst* InstNode: *(m_dfg->getInstNodes())){
		delete m_mapInfo[InstNode];
	}
}
