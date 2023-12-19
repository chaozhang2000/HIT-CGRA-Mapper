#include "DFGNodeInst.h"
#include "common.h"

DFGNodeInst::DFGNodeInst(int t_id,Instruction*t_inst,string t_name):DFGNode(t_id,t_name){
	m_inst = t_inst;
	m_opcodeName = t_inst->getOpcodeName();
	m_numConst = 0;
}

Instruction* DFGNodeInst::getInst() {
  return m_inst;
}

string DFGNodeInst::getOpcodeName() {
  return m_opcodeName;
}

void DFGNodeInst::addConst() {
  ++m_numConst;
}
