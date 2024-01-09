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
		bool allPreInstNodeNotMapped(DFGNodeInst* t_InstNode);
		bool allPreInstNodeMapped(DFGNodeInst* t_InstNode);
		map<int,CGRANode*>* getMapPathforStartInstNode(DFGNodeInst* t_InstNode);
		bool getMapPathsandScheduleforInstNode(DFGNodeInst* t_InstNode);
		list<map<int,CGRANode*>*>* getPathsforInstNodetoCGRANode(DFGNodeInst* t_InstNode,CGRANode* cgraNode);
		map<int,CGRANode*>* Dijkstra_search(DFGNodeInst* t_srcDFGNode,DFGNodeInst* t_dstDFGNode,CGRANode* t_srcCGRANode,CGRANode* t_dstCGRANode);
		void updateSearchData(bool neighborisDst,CGRANode* node,CGRANode*neighbor,map<CGRANode*,int>*distance,map<CGRANode*,int>*timing,map<CGRANode*,CGRANode*>*previous,int linkaddcost);
		map<int,CGRANode*>* getmaincostPath(list<map<int,CGRANode*>*>* paths);

		bool schedule(map<int,CGRANode*>*path,DFGNodeInst* t_InstNode,bool t_IncludeDstCGRANode);

		void dumpPath(map<int,CGRANode*>*path);

  public:
		/**The constructor function of class MRRG 
		 */
		Mapper(DFG*t_dfg,CGRA* t_cgra,MRRG* t_mrrg);
		~Mapper();

		int getResMII();

		void heuristicMap();
};
#endif

