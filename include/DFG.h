/**
 * @file DFG.h
 * @author Cheng Tan 
 * @brief  the defination of DFG class
 * @version 0.1
 */

#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/IR/Use.h>
//#include <llvm/Analysis/CFG.h>
#include <llvm/Analysis/LoopInfo.h>
#include <list>
#include <set>
#include <map>
#include <iostream>

#include "DFGNodeInst.h"
#include "DFGEdge.h"

using namespace llvm;
using namespace std;
///the class of DFG
class DFG {
  private:
    int m_num;

		/**List to save the pointer of DFGEdges in DFG
		 */
    list<DFGEdge*> m_DFGEdges; 
															 
		/** 
		* this function is used to connect DFGNodes to generate DFG
		*/
    void connectDFGNodes();

    string changeIns2Str(Instruction* ins);

    DFGNodeInst* getNode(Value*);
		bool NodeIsInst(DFGNode*);

		/**Get the pointer of DFGEdge from t_src to t_dst DFGNode.The DFGEdge must be confirmed to have been created.You can use hasDFGEdge() to check this.
		 * @param t_src the pointer to the src DFGNode
		 * @param t_dst the pointer to the dst DFGNode
		 * @return DFGEdge* the pointer to DFGEdge we want to find.
		 */
    DFGEdge* getDFGEdge(DFGNode* t_src, DFGNode* t_dst);

		/**Check if the DFGEdge from t_src to t_dst DFGNode has be created
		 * @param t_src the pointer to the src DFGNode
		 * @param t_dst the pointer to the dst DFGNode
		 * @return true main the DFGEdge from t_src to t_dst is in m_DFGEdges,has been created in the past
		 */
    bool hasDFGEdge(DFGNode* t_src, DFGNode* t_dst);
    bool shouldIgnore(Instruction*);

  public:
		/**The value to record if error when construct DFG
		 */
		bool DFG_error;

		/**List to save the pointer of DFGNodes in DFG
		 */
    list<DFGNode*> nodes;

		/**The constructor function of class DFG
		 * @param t_F the function processed by functionpass
		 */
		DFG(Function& t_F);

		/**The destructor function of class DFG
		 */
		~DFG();

		/**Extract DFG from specific function 
 		* @param t_F the function pointer which the mapperPass is processing
 		*/
    void construct(Function&);

		/** Print the information of DFG.
		 * Print the name and number of occurrences of each operation that appears in the data flow graph
		 * Print the number of DFGNodes and DFGLinks in the DFG
		 */
    void showOpcodeDistribution();

		/** Generate the Dot file according the DFG
		 * we can latter use dot tool to generate the png file of DFG.
 		 * @param t_F : the function pointer which the mapperPass is processing
 		 * @param t_isTrimmedDemo : use to control the generation of different dot(png) files. If this value is true , the name of the DFGNode in the dot(png) is the opcode of the inst,but if this value is false ,the name will be the complete instruction. this value is given from the "isTrimmedDemo" param in param.json.
		 */
		void generateDot(Function &t_F, bool t_isTrimmedDemo);
};
