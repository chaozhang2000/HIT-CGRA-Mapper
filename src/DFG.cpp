#include "DFG.h"
#include <llvm/Analysis/LoopInfo.h>
#include <fstream>
#include "common.h"
#include <limits>
#include <cassert>

using namespace std;
/**
 * How this function is implemented:
 * 1. init some var
 * 2. use construct() function to construct DFG
 */
DFG::DFG(Function& t_F) {
	DFG_error = false;
  construct(t_F);
}

/**
 * How this function is implemented:
 * 1. delete the DFGNodes in nodes list
 * 2. delete the DFGEdges in m_DFGEdges
 */
DFG::~DFG() {
				list<DFGNodeInst*>::iterator inst_node_it;
				for (inst_node_it=m_InstNodes.begin();inst_node_it!=m_InstNodes.end();++inst_node_it){
					delete *inst_node_it;
				}
				list<DFGNodeConst*>::iterator const_node_it;
				for (const_node_it=m_ConstNodes.begin();const_node_it!=m_ConstNodes.end();++const_node_it){
					delete *const_node_it;
				}
				list<DFGEdge*>::iterator edge_it;
				for (edge_it=m_DFGEdges.begin();edge_it!=m_DFGEdges.end();++edge_it){
					delete *edge_it;
				}
}

/** Used to determine whether the instruction should be ignored(not add to the dfg)
 * @param t_inst instruction that need to be judged
 * @return true if the inst should be ignored
 */
bool DFG::shouldIgnore(Instruction* t_inst) {
	string ret = "ret";
	if(t_inst->getOpcodeName()== ret)return true;
	else return false;
}

 /** 
	* what is in his function
	* 1. Clear the data structions which is used to save nodes and edges
	* 2. make sure there has only one basic block in target function,and every inst has less than two operands
	* 3. create DFGNode for all inst
	* 4. create DFGEdge
	* 5. connect all DFGNode to generate the DFG
 */
void DFG::construct(Function& t_F) {

	//Clear the data structions which is used to save nodes and edges
  m_DFGEdges.clear();
  m_InstNodes.clear();
  int nodeID = 0;
  int dfgEdgeID = 0;
	

	//make sure this function has only one basic block
	if(t_F.getBasicBlockList().size()>1) {
		DFG_ERR("The II have more then one basic block!");
	}
	//make sure every inst has less than two operands
  for (BasicBlock::iterator II=t_F.begin()->begin(),IEnd=t_F.begin()->end(); II!=IEnd; ++II) {
    Instruction* curII = &*II;
		if (curII->getNumOperands()>2){
			DFG_ERR("The Inst have more then two operands, not support yet!\n"<<changeIns2Str(curII));
		}
	}


	// Construct dfg nodes
#ifdef CONFIG_DFG_DEBUG 
	OUTS("\nDFG DEBUG",ANSI_FG_BLUE); 
	OUTS("==================================",ANSI_FG_CYAN); 
  OUTS("[constructing DFG of target function: "<< t_F.getName().str()<<"]",ANSI_FG_CYAN);
#endif
  for (BasicBlock::iterator II=t_F.begin()->begin(),IEnd=t_F.begin()->end(); II!=IEnd; ++II) {
    Instruction* curII = &*II;
    // Ignore this IR if it is ret.
    if (shouldIgnore(curII)) {
      continue;
    }
    DFGNodeInst* dfgNodeInst;
#ifdef CONFIG_DFG_FULL_INST
    dfgNodeInst = new DFGNodeInst(nodeID++,curII,changeIns2Str(curII));
#else
    dfgNodeInst = new DFGNodeInst(nodeID++,curII,curII->getOpcodeName());
#endif
    m_InstNodes.push_back(dfgNodeInst);
#ifdef CONFIG_DFG_DEBUG 
    outs()<< *curII<<" -> (dfgNode ID: "<<dfgNodeInst->getID()<<")\n";
#endif
  }
  // Construct data flow edges.
  for (DFGNodeInst* nodeInst: m_InstNodes) {
    Instruction* curII = nodeInst->getInst();
        for (Instruction::op_iterator op = curII->op_begin(), opEnd = curII->op_end(); op != opEnd; ++op) {
					//the operands comes from other inst, need to create DFGEdge
          if (Instruction* tempInst = dyn_cast<Instruction>(*op)) {
            DFGEdge* dfgEdge;
              if (hasDFGEdge(getInstNode(tempInst), nodeInst))
                dfgEdge = getDFGEdge(getInstNode(tempInst), nodeInst);
              else {
                dfgEdge = new DFGEdge(dfgEdgeID++, getInstNode(tempInst), nodeInst);
                m_DFGEdges.push_back(dfgEdge);
              }
          } 
					//if the operand is a const
					else if(ConstantData* const_data = dyn_cast<ConstantData>(*op)){
							ConstantInt * const_int = dyn_cast<ConstantInt>(*op);
							if (const_int==NULL){
							DFG_ERR("The Inst's const is not Int, not support yet!\n"<<changeIns2Str(curII));
							}
							string name;
							raw_string_ostream stream(name);
							stream << *const_int;
							DFGNodeConst* constNode;
							constNode = new DFGNodeConst(nodeID++,const_data,stream.str());
							m_ConstNodes.push_back(constNode);
            	DFGEdge* dfgEdge;
               dfgEdge = new DFGEdge(dfgEdgeID++, constNode, nodeInst);
               m_DFGEdges.push_back(dfgEdge);
          } 
					//if the operand is a param
					else if (Argument* arg = dyn_cast<Argument>(op)){
							string name;
							raw_string_ostream stream(name);
							stream << *arg;
							DFGNodeParam* paramNode;
							paramNode = new DFGNodeParam(nodeID++,arg,stream.str());
							m_ParamNodes.push_back(paramNode);
            	DFGEdge* dfgEdge;
               dfgEdge = new DFGEdge(dfgEdgeID++, paramNode, nodeInst);
               m_DFGEdges.push_back(dfgEdge);
					}
					else{
							DFG_ERR("The Inst have unknow operand!\n"<<changeIns2Str(curII));
					}
        }
					}

  connectDFGNodes();
	reorderInstNodes();
}

void DFG::reorderInstNodes(){
	list<DFGNodeInst*> longestPath;
	// find the longest path in dfg
	DFS_findlongest(&longestPath);
#ifdef CONFIG_DFG_DEBUG 
	OUTS("==================================",ANSI_FG_CYAN); 
  OUTS("[Reorder Inst Nodes]",ANSI_FG_CYAN);
	outs()<<"longestPath: ";
	for(DFGNodeInst* InstNode:longestPath){
		outs()<<"Node"<<InstNode->getID()<<" -> ";
	}
	outs()<<"end\n";
#endif
#ifdef CONFIG_DFG_LONGEST
	changeLongestPathColor(&longestPath,"orange");
#endif

	set<DFGNodeInst*> havenotSetLevelNodes;
	int maxlevel = 0;
	//set level for nodes in longestPath
	maxlevel =setLevelLongestPath(&longestPath,&havenotSetLevelNodes) ;

	//set level for other node
	setLevelforOtherNodes(&havenotSetLevelNodes);
	//reorder the InstNode in DFG
	list<DFGNodeInst*> temp;
	for(int i = 0;i<=maxlevel;i++){
		for(DFGNodeInst* InstNode: m_InstNodes){
			if(InstNode->getLevel() == i){
				temp.push_back(InstNode);
			}
		}
	}
	m_InstNodes.clear();
	for(DFGNodeInst* InstNode: temp){
		m_InstNodes.push_back(InstNode);
#ifdef CONFIG_DFG_DEBUG 
		outs()<<"Node"<<InstNode->getID()<<" "<<" level: "<< InstNode->getLevel()<<*(InstNode->getInst())<<"\n";
#endif
	}
}

DFGEdge* DFG::getEdgefrom(DFGNodeInst* t_src,DFGNodeInst* t_dst){
	for(DFGEdge* edge: m_DFGEdges){
		if(dynamic_cast<DFGNodeInst*>(edge->getSrc()) == t_src and dynamic_cast<DFGNodeInst*>(edge->getDst())== t_dst){
			return edge;
		}
	}
	assert("ERROR cannot find the DFGEdge from src to dst");
	return NULL;
}

/** set Level for the InstNode on the longest Path in DFG,and erase them from havenotSetLevelNodes list
*/
int DFG::setLevelLongestPath(list<DFGNodeInst*>*longestPath,set<DFGNodeInst*>* havenotSetLevelNodes){
	int level = 0;
	for(DFGNodeInst* InstNode: m_InstNodes){
					havenotSetLevelNodes->insert(InstNode);
	}
	// set level for node in the longest path
	for(DFGNodeInst* InstNode: *longestPath){
		InstNode -> setLevel(level);
		havenotSetLevelNodes->erase(InstNode);
		level ++;
	}
	return level;
}

/** set Level for the InstNode not on the longest Path in DFG,and erase them from havenotSetLevelNodes list
 * 1.search all nodes which have not been set level,and get four kinds of nodes,(1)nodes where all succnodes have level.(2)nodes where all prednodes have level.(3)node has highest proportion of succnodes which has been set(4)node has highest proportion of prednodes which has been set
 * 2.set level for (1)nodes,then search all nodes to get four kinds of nodes again.if kind (1) nodes not exsist,then set level for kind(2) nodes,and so on.
*/
void DFG::setLevelforOtherNodes(set<DFGNodeInst*>* havenotSetLevelNodes){
		DFGNodeInst* mostSuccNodeHasLevel;
		DFGNodeInst* mostPredNodeHasLevel;
		set<DFGNodeInst*> allSuccNodeHasLevel;
		set<DFGNodeInst*> allPredNodeHasLevel;
		float succNodeHasLevelPercent=0;
		float predNodeHasLevelPercent=0;
	while(havenotSetLevelNodes->size() > 0){
		allSuccNodeHasLevel.clear();
		allPredNodeHasLevel.clear();
		mostSuccNodeHasLevel = NULL;
		mostSuccNodeHasLevel = NULL;
		succNodeHasLevelPercent = 0;
		predNodeHasLevelPercent = 0;
		//find four kinds InstNodes
		for(DFGNodeInst* InstNode:*havenotSetLevelNodes){
						bool flag = true;
						int succnotfindcnt = 0;
						for(DFGNodeInst* succNode:*(InstNode->getSuccInstNodes())){
							if(havenotSetLevelNodes->find(succNode) != havenotSetLevelNodes->end()){
											flag = false;
											succnotfindcnt ++;
							}
						}
						if(flag == true)allSuccNodeHasLevel.insert(InstNode);
						if(succNodeHasLevelPercent < (InstNode->getSuccInstNodes()->size()==0)?0:1-((float)succnotfindcnt)/((float)(InstNode->getSuccInstNodes()->size()))){
							succNodeHasLevelPercent = (InstNode->getSuccInstNodes()->size()==0)?0:1-((float)succnotfindcnt)/((float)(InstNode->getSuccInstNodes()->size()));
							mostSuccNodeHasLevel = InstNode;
						}
						

						flag = true;
						int prednotfindcnt = 0;
						for(DFGNodeInst* predNode:*(InstNode->getPredInstNodes())){
							if(havenotSetLevelNodes->find(predNode) != havenotSetLevelNodes->end()){
											flag = false;
											prednotfindcnt ++;					
							}
						}
						if(flag == true)allPredNodeHasLevel.insert(InstNode);
						if(predNodeHasLevelPercent < (InstNode->getPredInstNodes()->size()==0)?0:1-((float)prednotfindcnt)/((float)(InstNode->getPredInstNodes()->size()))){
							predNodeHasLevelPercent = (InstNode->getPredInstNodes()->size()==0)?0:1-((float)prednotfindcnt)/((float)(InstNode->getPredInstNodes()->size()));
							mostPredNodeHasLevel = InstNode;
						}
		}
		//give kind(1) nodes level
		if(allSuccNodeHasLevel.size() > 0){
						for(DFGNodeInst* InstNode:allSuccNodeHasLevel){
							int level = numeric_limits<int>::max();	
							for(DFGNodeInst* succNode:*(InstNode->getSuccInstNodes())){
								if(level > succNode->getLevel()-1) level = succNode->getLevel() - 1;
							}
							InstNode->setLevel(level);
							havenotSetLevelNodes->erase(InstNode);
						}
			continue;
		//give kind(2) nodes level
		}else if (allPredNodeHasLevel.size()>0){
						for(DFGNodeInst* InstNode:allPredNodeHasLevel){
							int level = 0;	
							for(DFGNodeInst* predNode:*(InstNode->getPredInstNodes())){
								if(level < predNode->getLevel()+1) level = predNode->getLevel() + 1;
							}
							InstNode->setLevel(level);
							havenotSetLevelNodes->erase(InstNode);
						}
			continue;
		//give kind(3) node level
		}else if(mostSuccNodeHasLevel !=NULL){
							int level = numeric_limits<int>::max();	
							for(DFGNodeInst* succNode:*(mostSuccNodeHasLevel->getSuccInstNodes())){
								if(level > succNode->getLevel()-1 and succNode->haveSetLevel()) level = succNode->getLevel() - 1;
							}
							mostPredNodeHasLevel->setLevel(level);
							havenotSetLevelNodes->erase(mostPredNodeHasLevel);
			continue;
		//give kind(4) node level
		}else if(mostPredNodeHasLevel !=NULL){
							int level = 0;
							for(DFGNodeInst* predNode:*(mostPredNodeHasLevel->getPredInstNodes())){
								if(level < predNode->getLevel()+1) level = predNode->getLevel() + 1;
							}
							mostPredNodeHasLevel->setLevel(level);
							havenotSetLevelNodes->erase(mostPredNodeHasLevel);
			continue;
		}else{
						assert("ERROR has DFGNode which do not have INEdge and OutEdge");
			continue;
		}
	}
}

/** change the DFGEdge's color in Longest Path in DFG
*/
void DFG::changeLongestPathColor(list<DFGNodeInst*>* t_longestPath,string t_color){
	list<DFGNodeInst*>::iterator itr = t_longestPath->begin();
	DFGNodeInst* src = *itr;
	DFGNodeInst* dst = NULL;
	DFGEdge* edge = NULL;
	for(auto it=next(itr);it != t_longestPath->end();++it){
					dst = *it;
					edge = getEdgefrom(src,dst);
					edge->setcolor(t_color);
					src = *it;
	}
}
 /** 
	* get longest Path in DFG.
	* what is in his function
	* Traverse all nodes, treat each node as the initial node, and use DFS to find the longest path from the starting node in DFG. Save the longest path starting from different starting nodes.
	*
 */
void DFG::DFS_findlongest(list<DFGNodeInst*>* t_longestPath){
	list<DFGNodeInst*> currentPath;
	set <DFGNodeInst*> visited;
	for(DFGNodeInst* InstNode:m_InstNodes){
		currentPath.clear();
		visited.clear();
		reorderDFS(&visited,t_longestPath,&currentPath,InstNode);
	}
}

 /** 
	* get longest Path in DFG from start node.
	* use DFS to find the longest path from the starting node in DFG.
	* DFS(Depth-First Search) algorithm is an algorithm used to traverse or search for data in graphs. It starts from the starting node and follows a path as deep as possible into each previously unreachable node in the graph until it reaches the deepest node. Then, go back to the previous node and continue exploring other branches until all nodes are accessed.
 */
void DFG::reorderDFS(set<DFGNodeInst*>* t_visited, list<DFGNodeInst*>* t_targetPath,
								list<DFGNodeInst*>* t_curPath, DFGNodeInst* targetInstNode){
	t_visited->insert(targetInstNode);	
	t_curPath->push_back(targetInstNode);
	if(t_curPath->size() > t_targetPath->size()){
		t_targetPath->clear();
		for(DFGNodeInst* InstNode: *t_curPath){
			t_targetPath->push_back(InstNode);
		}
	}
	for(DFGNodeInst* succNode: *(targetInstNode->getSuccInstNodes())){
		if(t_visited->find(succNode) == t_visited->end()){
			reorderDFS(t_visited,t_targetPath,t_curPath,succNode);
			t_visited->erase(succNode);
			t_curPath->pop_back();
		}
	}
}

 /** 
	* what is in this function:
 	* Traverse all DFGEdges ,setOutEdge for the src DFGNode and setInEdeg for the dst DFGNode
 	*/
void DFG::connectDFGNodes() {
  for (DFGEdge* edge: m_DFGEdges) {
    DFGNode* left = edge->getSrc();
    DFGNode* right = edge->getDst();
    left->setOutEdge(edge);
    right->setInEdge(edge);
  }
}

void DFG::generateDot(Function &t_F, bool t_isTrimmedDemo) {
  error_code error;
//  sys::fs::OpenFlags F_Excl;
  string func_name = t_F.getName().str();
  string file_name = func_name + ".dot";
  StringRef fileName(file_name);
  raw_fd_ostream file(fileName, error, sys::fs::OF_None);

  file << "digraph \"DFG for'"  << t_F.getName() << "\' function\" {\n";

  //Dump InstDFG nodes.
  for (DFGNodeInst* node: m_InstNodes) {
      file << "\tNode" << node->getID() << "[shape=record,"<<"color="<<node->color<<",label=\"" << "(" << node->getID() << ") " << node->getName();
#ifdef CONFIG_DFG_LEVEL
			file << " level="<<node->getLevel();
#endif
		 file	<< "\"];\n";
		}
  //Dump ConstDFG nodes.
  for (DFGNodeConst* node: m_ConstNodes) {
      file << "\tNode" << node->getID() <<  "[shape=record,"<<"color="<<node->color<<",label=\"" << "(" << node->getID() << ") " << node->getName() << "\"];\n";
		}
  //Dump ParamDFG nodes.
  for (DFGNodeParam* node: m_ParamNodes) {
      file << "\tNode" << node->getID() <<  "[shape=record,"<<"color="<<node->color<<",label=\"" << "(" << node->getID() << ") " << node->getName() << "\"];\n";
		}
  // Dump data flow.
  for (DFGEdge* edge: m_DFGEdges) {
    // Distinguish data and control flows. Make ctrl flow invisible.
        file << "\tNode" << edge->getSrc()->getID() << " -> Node" << edge->getDst()->getID() << " [color="<< edge->getcolor()<<"]" << "\n";
  }
  file << "}\n";
  file.close();

}

void DFG::showOpcodeDistribution() {

	OUTS("==================================",ANSI_FG_CYAN); 
  OUTS("[show opcode count]",ANSI_FG_CYAN);
  map<string, int> opcodeMap;
  for (DFGNodeInst* node: m_InstNodes) {
    opcodeMap[node->getOpcodeName()] += 1;
  }
  for (map<string, int>::iterator opcodeItr=opcodeMap.begin();
      opcodeItr!=opcodeMap.end(); ++opcodeItr) {
    outs()<< (*opcodeItr).first << " : " << (*opcodeItr).second << "\n";
  }
  outs()<< "Inst DFG node count: "<<m_InstNodes.size()<<";\n";
  outs()<< "Const DFG node count: "<<m_ConstNodes.size()<<";\n";
  outs()<< "Param DFG node count: "<<m_ParamNodes.size()<<";\n";
	outs()<< "DFG edge count: "<<m_DFGEdges.size()<<";\n";
}

DFGNodeInst* DFG::getInstNode(Value* t_value) {
  for (DFGNode* node: m_InstNodes) {
					DFGNodeInst* nodeInst = dynamic_cast<DFGNodeInst*>(node);
					if(nodeInst&&nodeInst->getInst() == t_value){
      return nodeInst;
					}
  }
  assert("ERROR cannot find the corresponding DFG node.");
  return NULL;
}

DFGEdge* DFG::getDFGEdge(DFGNode* t_src, DFGNode* t_dst) {
  for (DFGEdge* edge: m_DFGEdges) {
    if (edge->getSrc() == t_src and
        edge->getDst() == t_dst) {
      return edge;
    }

  }
  assert("ERROR cannot find the corresponding DFG edge.");
  return NULL;
}

bool DFG::hasDFGEdge(DFGNode* t_src, DFGNode* t_dst) {
  for (DFGEdge* edge: m_DFGEdges) {
    if (edge->getSrc() == t_src and
        edge->getDst() == t_dst) {
      return true;
    }
  }
  return false;
}

string DFG::changeIns2Str(Instruction* t_ins) {
  string temp_str;
  raw_string_ostream os(temp_str);
  os << *t_ins;
  return os.str();
}

int DFG::getInstNodeCount(){
	return m_InstNodes.size();
}
