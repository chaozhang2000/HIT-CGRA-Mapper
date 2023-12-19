/**
 * @file DFG.cpp
 * @author Cheng Tan 
 * @brief  the implementation of DFG class
 * @version 0.1
 */
#include <fstream>
#include "DFG.h"
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
				list<DFGNode*>::iterator node_it;
				for (node_it=nodes.begin();node_it!=nodes.end();++node_it){
					delete *node_it;
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
  nodes.clear();
  int nodeID = 0;
  int dfgEdgeID = 0;
	

	//make sure this function has only one basic block
	if(t_F.getBasicBlockList().size()>1) {
		DFG_error = true;
		outs() <<ANSI_FMT("The II have more then one basic block!", ANSI_FG_RED)<<"\n";
		return;
	}
	//make sure every inst has less than two operands
  for (BasicBlock::iterator II=t_F.begin()->begin(),IEnd=t_F.begin()->end(); II!=IEnd; ++II) {
    Instruction* curII = &*II;
		if (curII->getNumOperands()>2){
			DFG_error = true;
			outs() <<ANSI_FMT("The Inst have more then two operands, not support yet!", ANSI_FG_RED)<<"\n";
			outs() <<ANSI_FG_RED<<changeIns2Str(curII)<<ANSI_NONE<<"\n";
			return;
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
    DFGNodeInst* dfgNode;
    dfgNode = new DFGNodeInst(nodeID++,curII,curII->getOpcodeName());
    nodes.push_back(dfgNode);
#ifdef CONFIG_DFG_DEBUG 
    outs()<< *curII<<" -> (dfgNode ID: "<<dfgNode->getID()<<")\n";
#endif
  }
  // Construct data flow edges.
  for (DFGNode* node: nodes) {
					DFGNodeInst* nodeInst = dynamic_cast<DFGNodeInst*>(node);
					if(nodeInst){
    Instruction* curII = nodeInst->getInst();
        for (Instruction::op_iterator op = curII->op_begin(), opEnd = curII->op_end(); op != opEnd; ++op) {
          Instruction* tempInst = dyn_cast<Instruction>(*op);
					//the operands comes from other inst, need to create DFGEdge
          if (tempInst) {
            DFGEdge* dfgEdge;
              if (hasDFGEdge(getNode(tempInst), node))
                dfgEdge = getDFGEdge(getNode(tempInst), node);
              else {
                dfgEdge = new DFGEdge(dfgEdgeID++, getNode(tempInst), node);
                m_DFGEdges.push_back(dfgEdge);
              }
          } 
					//if the operand is a const
					//else if(ConstantInt *C = dyn_cast<ConstantInt>(*op)){
					else if(dyn_cast<ConstantInt>(*op)){
              nodeInst->addConst();
							//outs() << C->getSExtValue() << "\n";
							//outs()<< *curII <<"\n";
          } 
					//if the operand is a param
					else{
							//outs()<<"is a param"<<"\n";
							//outs()<<*curII<<"\n";
              nodeInst->addConst();
					
					}
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

  //Dump DFG nodes.
  for (DFGNode* node: nodes) {
		if(NodeIsInst(node)){
//    if (dyn_cast<Instruction>((*node)->getInst())) {
      file << "\tNode" << node->getID() << node->getName() << "[shape=record, label=\"" << "(" << node->getID() << ") " << node->getName() << "\"];\n";
		}
  }
  // Dump data flow.
  file << "edge [color=red]" << "\n";
  for (DFGEdge* edge: m_DFGEdges) {
    // Distinguish data and control flows. Make ctrl flow invisible.
        file << "\tNode" << edge->getSrc()->getID() << edge->getSrc()->getName() << " -> Node" << edge->getDst()->getID() << edge->getDst()->getName() << "\n";
  }
  file << "}\n";
  file.close();

}

void DFG::showOpcodeDistribution() {

	OUTS("==================================",ANSI_FG_CYAN); 
  OUTS("[show opcode count]",ANSI_FG_CYAN);
  map<string, int> opcodeMap;
  for (DFGNode* node: nodes) {
    opcodeMap[node->getName()] += 1;
  }
  for (map<string, int>::iterator opcodeItr=opcodeMap.begin();
      opcodeItr!=opcodeMap.end(); ++opcodeItr) {
    outs()<< (*opcodeItr).first << " : " << (*opcodeItr).second << "\n";
  }
  outs()<< "DFG node count: "<<nodes.size()<<"; DFG edge count: "<<m_DFGEdges.size()<<";\n";
}

bool DFG::NodeIsInst(DFGNode*node){
if(dynamic_cast<DFGNodeInst*>(node)){
				return true;
}
return false;
}
DFGNodeInst* DFG::getNode(Value* t_value) {
  for (DFGNode* node: nodes) {
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
  t_ins->print(os);
  return os.str();
}
