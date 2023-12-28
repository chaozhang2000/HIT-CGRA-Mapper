#ifndef CGRA_H
#define CGRA_H
#include "CGRANode.h"
#include "CGRALink.h"

class CGRA {
  private:
		/** the var to save the num of CGRAlinks in the CGRA
		 */
    int m_FUCount;

		/** the var to save the num of CGRAlinks in the CGRA
		 */
    int m_LinkCount;

		/** the var to save the rows of CGRAlinks in the CGRA
		 */
    int m_rows;

		/** the var to save the columns of CGRAlinks in the CGRA
		 */
    int m_columns;

  public:
		
		/**The constructor function of class CGRA
		 */
		CGRA(int t_rows,int t_columns);
    CGRANode ***nodes;
    CGRALink **links;
};
#endif
