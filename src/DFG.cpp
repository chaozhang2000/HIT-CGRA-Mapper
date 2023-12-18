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
		errs() <<ANSI_FMT("The II have more then one basic block!", ANSI_FG_RED)<<"\n";
		return;
	}
	//make sure every inst has less than two operands
  for (BasicBlock::iterator II=t_F.begin()->begin(),IEnd=t_F.begin()->end(); II!=IEnd; ++II) {
    Instruction* curII = &*II;
		if (curII->getNumOperands()>2){
			DFG_error = true;
			errs() <<ANSI_FMT("The Inst have more then two operands, not support yet!", ANSI_FG_RED)<<"\n";
			errs() <<ANSI_FG_RED<<changeIns2Str(curII)<<ANSI_NONE<<"\n";
			return;
		}
	}


	// Construct dfg nodes
#ifdef CONFIG_DFG_DEBUG 
	ERRS("==================================",ANSI_FG_CYAN); 
  ERRS("[constructing DFG of target function: "<< t_F.getName().str()<<"]",ANSI_FG_CYAN);
#endif
  for (BasicBlock::iterator II=t_F.begin()->begin(),IEnd=t_F.begin()->end(); II!=IEnd; ++II) {
    Instruction* curII = &*II;
    // Ignore this IR if it is ret.
    if (shouldIgnore(curII)) {
      continue;
    }
    DFGNode* dfgNode;
    dfgNode = new DFGNode(nodeID++,  curII);
    nodes.push_back(dfgNode);
#ifdef CONFIG_DFG_DEBUG 
    errs()<< *curII<<" -> (dfgNode ID: "<<dfgNode->getID()<<")\n";
#endif
  }
  // Construct data flow edges.
  for (DFGNode* node: nodes) {
    Instruction* curII = node->getInst();
    assert(node == getNode(curII));
        for (Instruction::op_iterator op = curII->op_begin(), opEnd = curII->op_end(); op != opEnd; ++op) {
          Instruction* tempInst = dyn_cast<Instruction>(*op);
          if (tempInst) {
            DFGEdge* dfgEdge;
              if (hasDFGEdge(getNode(tempInst), node))
                dfgEdge = getDFGEdge(getNode(tempInst), node);
              else {
                dfgEdge = new DFGEdge(dfgEdgeID++, getNode(tempInst), node);
                m_DFGEdges.push_back(dfgEdge);
              }
          } 
					else {
              node->addConst();
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
//    if (dyn_cast<Instruction>((*node)->getInst())) {
    if (t_isTrimmedDemo) {
      file << "\tNode" << node->getID() << node->getOpcodeName() << "[shape=record, label=\"" << "(" << node->getID() << ") " << node->getOpcodeName() << "\"];\n";
    } else {
      file << "\tNode" << node->getInst() << "[shape=record, label=\"" <<
          changeIns2Str(node->getInst()) << "\"];\n";
    }
  }
  // Dump data flow.
  file << "edge [color=red]" << "\n";
  for (DFGEdge* edge: m_DFGEdges) {
    // Distinguish data and control flows. Make ctrl flow invisible.
      if (t_isTrimmedDemo) {
        file << "\tNode" << edge->getSrc()->getID() << edge->getSrc()->getOpcodeName() << " -> Node" << edge->getDst()->getID() << edge->getDst()->getOpcodeName() << "\n";
      } else {
        file << "\tNode" << edge->getSrc()->getInst() << " -> Node" << edge->getDst()->getInst() << "\n";
      }
  }
  file << "}\n";
  file.close();

}

void DFG::showOpcodeDistribution() {

	ERRS("==================================",ANSI_FG_CYAN); 
  ERRS("[show opcode count]",ANSI_FG_CYAN);
  map<string, int> opcodeMap;
  for (DFGNode* node: nodes) {
    opcodeMap[node->getOpcodeName()] += 1;
  }
  for (map<string, int>::iterator opcodeItr=opcodeMap.begin();
      opcodeItr!=opcodeMap.end(); ++opcodeItr) {
    errs()<< (*opcodeItr).first << " : " << (*opcodeItr).second << "\n";
  }
  errs()<< "DFG node count: "<<nodes.size()<<"; DFG edge count: "<<m_DFGEdges.size()<<";\n";
}

DFGNode* DFG::getNode(Value* t_value) {
  for (DFGNode* node: nodes) {
    if (node->getInst() == t_value) {
      return node;
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
