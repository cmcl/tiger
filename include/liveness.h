#ifndef TIGER_LIVENESS_H_
#define TIGER_LIVENESS_H_

#include "graph.h"
#include "temp.h"

typedef struct Live_moveList_ *Live_moveList;
struct Live_moveList_ {
	G_node src, dst;
	Live_moveList tail;
};

struct Live_graph {
	G_graph graph;
	Live_moveList moves;
};

Live_moveList Live_MoveList(G_node src, G_node dst, Live_moveList tail);
struct Live_graph Live_liveness(G_graph flow);
Temp_temp Live_gtemp(G_node node);

#endif // TIGER_LIVENESS_H_
