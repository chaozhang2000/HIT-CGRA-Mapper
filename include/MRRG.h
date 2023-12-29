#ifndef MRRG_H
#define MRRG_H

#include "CGRA.h"
#include <map>
using namespace std;

enum {SRC_NOT_OCCUPY,SRC_OCCUPY_FROM_N, SRC_OCCUPY_FROM_S,SRC_OCCUPY_FROM_W,SRC_OCCUPY_FROM_E,SRC_OCCUPY_FROM_FU,SRC_OCCUPY_FROM_CONST_MEM,SRC_OCCUPY_FROM_LOOP0,SRC_OCCUPY_FROM_LOOP1,SRC_OCCUPY_FROM_LOOP2};
enum {LINK_NOT_OCCUPY,LINK_OCCUPY_FROM_N, LINK_OCCUPY_FROM_S,LINK_OCCUPY_FROM_W,LINK_OCCUPY_FROM_E,LINK_OCCUPY_FROM_FU};
class MRRG {
  private:
		class NodeInfo{
			public:
		/** this var is used to record the CGRANode is occupyed by a DFGNode at certain cycle
		 * m_OccupyedByNode = new DFGNodeInst*[cycle];
		 */
		DFGNodeInst** m_OccupiedByNode;

		/**record if the Src1 or Src2 input mux is occpuied at a certain cycle,and it's occupied state
		 * for example
		 * SRC_OCCUPY_N,mean the mux is occupy and the src1 data is come from the input CGRALink from north
		 * SRC_OCCUPY_FU,mean the CGRALink is occupy and the output data is come from the fu out last time
		 */
		int *m_Src1OccupyState;
		int *m_Src2OccupyState;
		};
		class LinkInfo{
			public:
		/**record if the CGRALink is occpuied at a certain cycle,and it's occupied state
		 * for example
		 * LINK_OCCUPY_N,mean the CGRALink is occupy and the output data is come from the input from north
		 * LINK_OCCUPY_FU,mean the CGRALink is occupy and the output data is come from the fu out
		 */
		int *m_occupied_state;
		};
		CGRA* m_cgra;

		int m_cycles;

		map<CGRANode*,NodeInfo*> m_NodeInfos;
		map<CGRALink*,LinkInfo*> m_LinkInfos;
  public:
		/**The constructor function of class MRRG 
		 */
		MRRG(CGRA* t_cgra,int m_cycles);
		~MRRG();
};
#endif

