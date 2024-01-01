#ifndef Mapper_H
#define Mapper_H

#include "CGRA.h"
#include "DFG.h"
#include "MRRG.h"
using namespace std;

class Mapper{
  private:
		CGRA* m_cgra;

		DFG* m_dfg;

		MRRG* m_mrrg;

		int m_II;

		/**The class to record the map info, cgra record which CGRANode the DFGNodeInst is mapped to ,and the cycle record which clock is mapped to.
		 */
		class MapInfo{
			public:
				CGRANode* cgra;
				int cycle;
				bool mapped;
		};
		map<DFGNodeInst*,MapInfo*> m_mapInfo;

		void mapInfoInit();
		void getMapPathsforInstNode(DFGNodeInst* t_InstNode,list<map<CGRANode*,int>*>* t_paths);
		map<CGRANode*,int>* getPathforInstNodetoCGRANode(DFGNodeInst* t_InstNode,CGRANode* t_cgraNode);

  public:
		/**The constructor function of class MRRG 
		 */
		Mapper(DFG*t_dfg,CGRA* t_cgra,MRRG* t_mrrg);
		~Mapper();

		int getResMII();

		void heuristicMap();
};
#endif

