/**
 * @file DFGNode.cpp
 * @author Cheng Tan 
 * @brief  the implementation of DFGNode class
 * @version 0.1
 */

#include "DFGNode.h"
#include "common.h"

/**what is in this function:
 * init the DFGNode for example init the id of DFGNode(m_id) = t_id
 * get the OpcodeName form the t_inst and init the m_opcodeName and so on
 *
*/
DFGNode::DFGNode(int t_id, Instruction* t_inst) {
  m_id = t_id;
  m_inst = t_inst;
  m_opcodeName = t_inst->getOpcodeName();
  m_numConst = 0;
}

int DFGNode::getID() {
  return m_id;
}

Instruction* DFGNode::getInst() {
  return m_inst;
}

string DFGNode::getOpcodeName() {
  return m_opcodeName;
}

void DFGNode::setInEdge(DFGEdge* t_dfgEdge) {
  if (find(m_inEdges.begin(), m_inEdges.end(), t_dfgEdge) ==
      m_inEdges.end())
    m_inEdges.push_back(t_dfgEdge);
}

void DFGNode::setOutEdge(DFGEdge* t_dfgEdge) {
  if (find(m_outEdges.begin(), m_outEdges.end(), t_dfgEdge) ==
      m_outEdges.end())
    m_outEdges.push_back(t_dfgEdge);
}

void DFGNode::addConst() {
  ++m_numConst;
}

