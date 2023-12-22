#ifndef DFGNodeConst_H
#define DFGNodeConst_H
#include <llvm/IR/Constant.h>
#include "DFGNode.h"

using namespace llvm;
using namespace std;

class DFGNodeConst:public DFGNode{
	private:
		ConstantData* m_const;
	public:
		static const string color;
		DFGNodeConst(int t_id, ConstantData* t_const,string t_name);
		~DFGNodeConst(){};

    ConstantData* getConstant();
};
#endif
