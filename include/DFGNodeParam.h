#ifndef DFGNodeParam_H
#define DFGNodeParam_H
#include <llvm/IR/Argument.h>
#include "DFGNode.h"

using namespace llvm;
using namespace std;

class DFGNodeParam:public DFGNode{
	private:
		Argument* m_param;
	public:
		static const string color;
		DFGNodeParam(int t_id, Argument* t_param,string t_name);
		~DFGNodeParam(){};

    Argument* getParam();
};
#endif
