#ifndef DFGNodeInst_H
#define DFGNodeInst_H
#include <llvm/IR/Value.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/raw_ostream.h>

#include <string>
#include <list>
#include <stdio.h>
#include <iostream>
#include "DFGNode.h"

using namespace llvm;
using namespace std;

class DFGNodeInst:public DFGNode{
	private:
		Instruction* m_inst;
		string m_opcodeName;	
		/**this variable is used to record how many const there are in the operands of the current DFG node
		 */
    int m_numConst;
	public:
		/**The constructor function of class DFGNodeInst
		 * @param t_id :the id that give the DFGNode
		 * @param t_inst :The instruction corresponding to this DFGNodeInst
		 */
		DFGNodeInst(int t_id, Instruction* t_inst,string t_name);
		~DFGNodeInst(){};

    Instruction* getInst();

    string getOpcodeName();

		/** m_numConst + 1
		 */
    void addConst();
	
};
#endif
