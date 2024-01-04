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
				CGRANode* cgraNode;
				int cycle;
				bool mapped;
		};
		map<DFGNodeInst*,MapInfo*> m_mapInfo;

		void mapInfoInit();
		void getMapPathsforInstNode(DFGNodeInst* t_InstNode,list<map<CGRANode*,int>*>* t_paths);
		map<CGRANode*,int>* getPathforInstNodetoCGRANode(DFGNodeInst* t_InstNode,CGRANode* t_cgraNode);
		map<CGRANode*,int>* Dijkstra_search(DFGNodeInst* t_srcDFGNode,DFGNodeInst* t_dstDFGNode,CGRANode* t_srcCGRANode,CGRANode* t_dstCGRANode);
		map<CGRANode*,int>* getmaincostPath(list<map<CGRANode*,int>*>* paths);

		bool schedule(map<CGRANode*,int>*mincostPath);

		void dumpPath(map<CGRANode*,int>*path);

  public:
		/**The constructor function of class MRRG 
		 */
		Mapper(DFG*t_dfg,CGRA* t_cgra,MRRG* t_mrrg);
		~Mapper();

		int getResMII();

		void heuristicMap();
};
#endif

