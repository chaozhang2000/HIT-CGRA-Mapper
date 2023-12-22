#include "DFGNodeParam.h"
#include "common.h"

const string DFGNodeParam::color= "red";
DFGNodeParam::DFGNodeParam(int t_id,Argument*t_param,string t_name):DFGNode(t_id,t_name){
	m_param = t_param;
}

Argument* DFGNodeParam::getParam() {
  return m_param;
}
