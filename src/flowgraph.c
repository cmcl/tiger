#include "flowgraph.h"
#include "assem.h"

Temp_tempList FG_def(G_node n);
{
	return NULL;
}

Temp_tempList FG_use(G_node n)
{
	return NULL;
}

bool FG_isMove(G_node n);
{
	AS_instr instr = G_nodeInfo(n);
	return (instr != NULL && instr->kind == I_MOVE);
}

G_graph FG_AssemFlowGraph(AS_instrList il)
{
	return NULL;
}
