/**
 * @file DFGNode.h
 * @author Cheng Tan 
 * @brief  the defination of DFGNode class
 * @version 0.1
 */
#ifndef DFGNode_H
#define DFGNode_H

#include <llvm/IR/Value.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/raw_ostream.h>

#include <string>
#include <list>
#include <stdio.h>
#include <iostream>

#include "DFGEdge.h"

using namespace llvm;
using namespace std;

class DFGEdge;

class DFGNode {
  private:
    int m_id;
    Instruction* m_inst;
    string m_opcodeName;
		/**the list to save the pointers of input DFGEdges
		 */
    list<DFGEdge*> m_inEdges;

		/**the list to save the pointers of output DFGEdges
		 */
    list<DFGEdge*> m_outEdges;

		/**this variable is used to record how many const there are in the operands of the current DFG node
		 */
    int m_numConst;

  public:
		/**The constructor function of class DFGNode
		 * @param t_id :the id that give the DFGNode
		 * @param t_precisionAware :TODO
		 * @param t_inst :The instruction corresponding to this DFGNode
		 */
		DFGNode(int t_id, Instruction* t_inst);

    int getID();

    Instruction* getInst();

    string getOpcodeName();

		/** add t_dfgEdge to m_inEdges of the DFGNode
		 *  @param t_dfgEdge : the pointer of DFGEdge to be added to the m_inEdges
		 */
    void setInEdge(DFGEdge* t_dfgEdge);
		/** add t_dfgEdge to m_outEdges of the DFGNode
		 *  @param t_dfgEdge : the pointer of DFGEdge to be added to the m_outEdges
		 */
    void setOutEdge(DFGEdge* t_dfgEdge);

		/** m_numConst + 1
		 */
    void addConst();
};
#endif
