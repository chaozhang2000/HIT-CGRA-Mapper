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
			list<map<int,CGRANode*>*> paths;
#ifdef CONFIG_MAP_DEBUG 
			outs()<<"\nTry to find paths for DFGNode"<<(*InstNode)->getID()<<" to each CGRANode\n"; 
#endif
			getMapPathsforInstNode(*InstNode,&paths);
			//if found some paths for this InstNode
			if(paths.size()!= 0){
				//find the mincostPath in paths
				map<int,CGRANode*>* mincostPath = getmaincostPath(&paths);

#ifdef CONFIG_MAP_DEBUG 
				outs()<<"Find mincost Path: ";
				dumpPath(mincostPath);
#endif
				//map the mincostPath in MRRG
				bool schedulesuccess = schedule(mincostPath,*InstNode);
				for(map<int,CGRANode*>* path : paths){delete path;}
				if(schedulesuccess){
#ifdef CONFIG_MAP_DEBUG 
					outs()<<"Success in schedule mincostPath\n";
#endif
				}
				else{
#ifdef CONFIG_MAP_DEBUG 
					outs()<<"Failed in schedule mincostPath\n";
#endif
					break;
				}
			}
			//if cant find a path for this InstNode
			else{
#ifdef CONFIG_MAP_DEBUG 
				outs()<<"Can't find path for InstNode"<<(*InstNode)->getID()<<"\nFaild mapping with II = "<<m_II<<"\n";
#endif
				//paths has no more use,delete
				for(map<int,CGRANode*>* path : paths){delete path;}
				break;
			}
		}
		m_II ++;
	}
}

/** this function is used to get all possible map paths for a DFGNode.
 *	it get paths to every CGRANodes, and save pathes in t_paths
 *	@param t_InstNode: the DFGNode need to be mapped
 *	@param t_paths: save the pathes
 */
void Mapper::getMapPathsforInstNode(DFGNodeInst* t_InstNode,list<map<int,CGRANode*>*>* t_paths){
	for(int r = 0; r< m_cgra->getrows();r++){
		for(int c = 0; c< m_cgra->getcolumns();c++){
			CGRANode* cgraNode = m_cgra->nodes[r][c];
			if(cgraNode->canSupport(t_InstNode->getOpcodeName()) and cgraNode->isdisable()==false){
				map<int,CGRANode*>* path = getPathforInstNodetoCGRANode(t_InstNode,cgraNode);
				if(path != NULL){
#ifdef CONFIG_MAP_DEBUG_PATH 
				outs()<<"Try to find path for DFGNode"<<t_InstNode->getID()<<" to CGRANode"<< cgraNode->getID()<<" success!\n  Path:";
				dumpPath(path);
#endif
					t_paths->push_back(path);
				}
				else{
#ifdef CONFIG_MAP_DEBUG_PATH 
				outs()<<"Try to find path for DFGNode"<<t_InstNode->getID()<<" to CGRANode"<< cgraNode->getID()<<" falied,can't find path!\n"; 
#endif
				}
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
map<int,CGRANode*>* Mapper::getPathforInstNodetoCGRANode(DFGNodeInst* t_InstNode,CGRANode* t_cgraNode){
			map<int,CGRANode*>* path = NULL;
			bool allPredNodenotMapped = true;
			int maxpathlastcycle = 0;
			list<map<int,CGRANode*>*> temppaths;//save the temppath, used to delete no use temppaths.
			for(DFGNodeInst* preInstNode: *(t_InstNode->getPredInstNodes())){
				if(m_mapInfo[preInstNode]->mapped == true){
					allPredNodenotMapped = false;
					//create a tmp path,to save the path from the pre cgra node to the t_cgra node
					//we only need to save the shortest path,delete other temppaths
					map<int,CGRANode*>* temppath = Dijkstra_search(preInstNode,t_InstNode,m_mapInfo[preInstNode]->cgraNode,t_cgraNode);
					//after try to find a path,if the Dijkstra_search return NULL,mean the path to t_cgraNode not exsist,because not every mapped preNode has path to t_cgraNode
					if(temppath == NULL){
						if(temppaths.size()!=0){
							for(map<int,CGRANode*>* deletepath: temppaths){
								delete deletepath;
							}
						}
						return NULL;
					//if the temmpath is found save it to the tmppaths,and find the longest temmpath and save to path.
					}else{
						temppaths.push_back(temppath);
						map<int,CGRANode*>::reverse_iterator rit = temppath->rbegin();
						if((*rit).first>= maxpathlastcycle){
							maxpathlastcycle = (*rit).first;
							path = temppath;
						}
					}
				}
			}
			//program reach here, has two situations,
			//1.the path has already found
			//2.allPredNodenotMapped == true,path not found
			//handle situation one
			if(path!=NULL){
				for(map<int,CGRANode*>* noUseTempPath: temppaths){
					if(noUseTempPath!=path){
								delete noUseTempPath;
					}
				}
				return path;
			}
			//handle situation two
			//TODO: should not be any CGRA node
			if(allPredNodenotMapped == true){
				int cycle = 0;
				path = new map<int,CGRANode*>;
				while(cycle <= m_mrrg->getMRRGcycles()){
					if(m_mrrg->canOccupyNode(t_cgraNode,cycle,m_II)==true){
						(*path)[cycle] = t_cgraNode;
						return path;
					}
					cycle ++;	
				}
			}
			return NULL;
}

/** this function try to find a path to a certain CGRANode for a DFGNode,when this DFGNode has some predNode which has been mapped.The path is from the CGRANode which map the preDFGNode to the CGRANode where the succDFGNode want to map
 * use dijkstara search to find this path,if find this path,return path,else return NULL
 */
map<int,CGRANode*>* Mapper::Dijkstra_search(DFGNodeInst* t_srcDFGNode,DFGNodeInst* t_dstDFGNode,CGRANode* t_srcCGRANode,CGRANode* t_dstCGRANode){
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
	map<int,CGRANode*>* path = NULL;
	if(successFindPath){
	 CGRANode* u = t_dstCGRANode;
	 path =  new map<int,CGRANode*>;
	 while(u!=NULL){
	 	(*path)[timing[u]] = u;
		u = previous[u];
	 }
	 //this is used to handle the special situation,
	 //when the succnode and the pre node are calculate in the same CGRANode,the unordered path just has one node
	 //but we also want the pre node's information, we want a path like CGRANode0(0)->CGRANode0(1),but not like CGRANode0(1)->
	 if(t_srcCGRANode == t_dstCGRANode){
	 	(*path)[m_mapInfo[t_srcDFGNode]->cycle] = t_dstCGRANode;
	 }
	}
	return path;
}

/** this function try to find a path with min cost,(choose a path to schedule)
 * TODO: now only consider the distance cost,more should be consider later
 */
map<int,CGRANode*>* Mapper::getmaincostPath(list<map<int,CGRANode*>*>* paths){
		map<int,CGRANode*>::reverse_iterator rit = paths->front()->rbegin();
		int mincost = (*rit).first;
		map<int,CGRANode*>* mincostpath = paths->front();
		for(map<int,CGRANode*>* path : *paths){
				rit = path->rbegin();
				if((*rit).first < mincost){
					mincostpath = path;
					mincost = (*rit).first;
				}
		}
		return mincostpath;
}

/** this function try to schedule the path in MRRG
 * @param t_InstNode: the dst DFGNode at the end of path
 */
bool Mapper::schedule(map<int,CGRANode*>*path,DFGNodeInst* t_InstNode){
	//schedule the Node.
	map<int,CGRANode*>::reverse_iterator ri = path->rbegin();
	CGRANode* dstCGRANode = (*ri).second;
#ifdef CONFIG_MAP_DEBUG_SCHEDULE 
			outs()<<"Schedule DFG node["<<t_InstNode->getID()<<"] onto CGRANode["<<dstCGRANode->getID()<<"] at cycle "<< (*ri).first <<" with II: "<<m_II<<"\n"; 
#endif
	m_mrrg->scheduleNode(dstCGRANode,t_InstNode,(*ri).first,m_II);
	m_mapInfo[t_InstNode]->cgraNode = dstCGRANode;
	m_mapInfo[t_InstNode]->cycle = (*ri).first;
	m_mapInfo[t_InstNode]->mapped = true;
	
	//schedule the Link.
	map<int,CGRANode*>::iterator it = path->begin();
	int cyclepre = (*it).first;
	int cyclecurrent = 0;
	CGRALink* currentLink = NULL;
	for(it = path->begin();it != path->end();it++){
			cyclecurrent = (*it).first;
			if(it != path->begin()){
				if((*path)[cyclepre]!=(*path)[cyclecurrent]){
					currentLink = m_cgra->getEdgefrom((*path)[cyclepre],(*path)[cyclecurrent]);
					int duration = cyclecurrent - cyclepre - 1;
#ifdef CONFIG_MAP_DEBUG_SCHEDULE 
					outs()<<"Schedule CGRALink["<<currentLink->getID()<<"] from CGRANode["<<(*path)[cyclepre]->getID()<<"] to CGRANode["<< (*path)[cyclecurrent]->getID()<<"] at cycle "<<cyclepre<<" to cycle "<<cyclepre + duration<<" with II = "<<m_II<<"\n"; 
#endif
					m_mrrg->scheduleLink(currentLink,cyclepre,duration,m_II);
				}
			}	
			cyclepre= cyclecurrent;
	}

	/*
	//route the mapped prenode 
	for(DFGNodeInst* preNode: *(t_InstNode->getPredInstNodes())){
		if(m_mapInfo[preNode]->mapped == true and m_mapInfo[preNode]->cgraNode
	}*/
	
	return true;
}

void Mapper::mapInfoInit(){
	for(DFGNodeInst* InstNode: *(m_dfg->getInstNodes())){
		m_mapInfo[InstNode]->cgraNode = NULL;
		m_mapInfo[InstNode]->cycle = 0;
		m_mapInfo[InstNode]->mapped = false;
	}
}

void Mapper::dumpPath(map<int,CGRANode*>*path){
	 for(map<int,CGRANode*>::iterator it =path->begin();it != path->end();it++) {
				outs()<<"CGRANode"<<(it->second)->getID()<<"("<<it->first<<")"<<"->";
	 }
	 outs()<<"\n";
}

Mapper::~Mapper(){
	for(DFGNodeInst* InstNode: *(m_dfg->getInstNodes())){
		delete m_mapInfo[InstNode];
	}
}

