
#include "DFGEdge.h"
#include "common.h"


const string DFGEdge::color= "black";
DFGEdge::DFGEdge(int t_id, DFGNode* t_src, DFGNode* t_dst) {
  m_id = t_id;
  m_src = t_src;
  m_dst = t_dst;
}

int DFGEdge::getID() {
  return m_id;
}

DFGNode* DFGEdge::getSrc() {
  return m_src;
}

DFGNode* DFGEdge::getDst() {
  return m_dst;
}

void DFGEdge::connect(DFGNode* t_src, DFGNode* t_dst) {
  m_src = t_src;
  m_dst = t_dst;
}
