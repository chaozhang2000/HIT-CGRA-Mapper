#ifndef DFGNodeInst_H
#define DFGNodeInst_H
#include <llvm/IR/Instruction.h>
#include "DFGNode.h"

using namespace llvm;
using namespace std;

class DFGNodeInst:public DFGNode{
	private:
		/**var to save the inst 
		 */
		Instruction* m_inst;
		/**var to save the opcode name of inst
		 */
		string m_opcodeName;	
		/**list to save the succInstNode
		 */
		list<DFGNodeInst*>* m_succInstNodes;
		/**list to save the succInstNode
		 */
		list<DFGNodeInst*>* m_predInstNodes;
		/**var to save the level of InstNode in DFG,the level will be given when reorder the DFG
		 * the dfgnode with lower level execute first
		 */
		int m_level;
		bool m_haveSetLevel;
	public:
		static const string color;
		/**The constructor function of class DFGNodeInst
		 * @param t_id :the id that give the DFGNode
		 * @param t_inst :The instruction corresponding to this DFGNodeInst
		 */
		DFGNodeInst(int t_id, Instruction* t_inst,string t_name);
		~DFGNodeInst();

    Instruction* getInst();

    string getOpcodeName();

		string getColor();

		list<DFGNodeInst*>* getSuccInstNodes();
		list<DFGNodeInst*>* getPredInstNodes();

		void setLevel(int t_level);

		int getLevel(){return m_level;};
		bool haveSetLevel(){return m_haveSetLevel;}
};
#endif
