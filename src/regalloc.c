#include "regalloc.h"
#include "color.h"
#include "graph.h"

struct RA_result RA_regAlloc(F_frame f, AS_instrList il)
{
	struct COL_result coloring = COL_color(G_empty(),
										F_tempMap, F_registers());
	struct RA_result result = {coloring.coloring, il};
	return result;
}
