#include "DFG.h"
#include <llvm/Support/FileSystem.h>
#include <llvm/Analysis/LoopInfo.h>
#include <fstream>
#include "common.h"
/**
 * How this function is implemented:
 * 1. init some var
 * 2. use construct() function to construct DFG
 */
DFG::DFG(Function& t_F) {
	DFG_error = false;
  m_num = 0;
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
              nodeInst->addConst();
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
              nodeInst->addConst();
					
					}
					else{
							DFG_ERR("The Inst have unknow operand!\n"<<changeIns2Str(curII));
					}
        }
					}

  connectDFGNodes();
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
      file << "\tNode" << node->getID() << "[shape=record,"<<"color="<<node->color<<",label=\"" << "(" << node->getID() << ") " << node->getName() << "\"];\n";
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
  file << "edge [color="<< m_DFGEdges.front()->color<<"]" << "\n";
  for (DFGEdge* edge: m_DFGEdges) {
    // Distinguish data and control flows. Make ctrl flow invisible.
        file << "\tNode" << edge->getSrc()->getID() << " -> Node" << edge->getDst()->getID() << "\n";
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
