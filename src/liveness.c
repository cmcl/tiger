#include "liveness.h"
#include "util.h"

Live_moveList Live_MoveList(G_node src, G_node dst, Live_moveList tail)
{
	Live_moveList moves = checked_malloc(sizeof(*moves));
	moves->src = src;
	moves->dst = dst;
	moves->tail = tail;
	return moves;
}

struct Live_graph Live_liveness(G_graph flow)
{
	struct Live_graph lg = {flow, NULL};
	return lg;
}

Temp_temp Live_gtemp(G_node node)
{
	return NULL;
}
