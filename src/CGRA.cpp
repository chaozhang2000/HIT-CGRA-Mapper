#include "CGRA.h"

/**
 * What is in this Function:
 * 1. init some var,like m_rows、m_columns、m_FUCount and so on.
 * 2. Depending on whether parameter a is true or false, decide whether to use paramCGRA.json or default parameters to initialize CGRA.
 * 3. init the CGRANode and CGRALink according to paramCGRA.json or default value.
 * 4. connect the CGRANode and CGRALink to Generate the CGRA.
 */
CGRA::CGRA(int t_rows,int t_columns){
	
  m_rows = t_rows;
  m_columns = t_columns;
  m_FUCount = t_rows * t_columns;
  nodes = new CGRANode**[t_rows];

  int node_id = 0;
  for (int i=0; i<t_rows; ++i) {
  	nodes[i] = new CGRANode*[t_columns];
    for (int j=0; j<t_columns; ++j) {
    	nodes[i][j] = new CGRANode(node_id++, j, i);
    }
  }
		//这里LinkCount的数量不再是由paramCGRA中来读取，而是采用下面的方式来进行计算，可以发现，这是一种默认的链接方式。
		//这是一种这样的连接方式:每个node的上下左右都双向连接，除了最外围的一圈只往里面连接
  m_LinkCount = 2 * (t_rows * (t_columns-1) + (t_rows-1) * t_columns);
  links = new CGRALink*[m_LinkCount];

    // Connect the CGRA nodes with links.
  int link_id = 0;
  for (int i=0; i<t_rows; ++i) {
  	for (int j=0; j<t_columns; ++j) {
			//right
			if (i < t_rows - 1) {
      	links[link_id] = new CGRALink(link_id);
        nodes[i][j]->attachOutLink(links[link_id]);
        nodes[i+1][j]->attachInLink(links[link_id]);
        links[link_id]->connect(nodes[i][j], nodes[i+1][j]);
        ++link_id;
			}
			//left
      if (i > 0) {
      	links[link_id] = new CGRALink(link_id);
        nodes[i][j]->attachOutLink(links[link_id]);
        nodes[i-1][j]->attachInLink(links[link_id]);
        links[link_id]->connect(nodes[i][j], nodes[i-1][j]);
        ++link_id;
      }
			//down
      if (j < t_columns - 1) {
        links[link_id] = new CGRALink(link_id);
        nodes[i][j]->attachOutLink(links[link_id]);
        nodes[i][j+1]->attachInLink(links[link_id]);
        links[link_id]->connect(nodes[i][j], nodes[i][j+1]);
        ++link_id;
      }
			//up
      if (j > 0) {
        links[link_id] = new CGRALink(link_id);
        nodes[i][j]->attachOutLink(links[link_id]);
        nodes[i][j-1]->attachInLink(links[link_id]);
        links[link_id]->connect(nodes[i][j], nodes[i][j-1]);
        ++link_id;
			}
    }
  }
}
