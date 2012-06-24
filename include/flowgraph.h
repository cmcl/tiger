#ifndef TIGER_FLOWGRAPH_H_
#define TIGER_FLOWGRAPH_H_
/*
 * flowgraph.h - Function prototypes to represent control flow graphs.
 */
#include "graph.h"
#include "temp.h"

Temp_tempList FG_def(G_node n);
Temp_tempList FG_use(G_node n);
bool FG_isMove(G_node n);
G_graph FG_AssemFlowGraph(AS_instrList il);

#endif // TIGER_FLOWGRAPH_H_
