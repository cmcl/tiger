#ifndef TIGER_COLOR_H_
#define TIGER_COLOR_H_
/*
 * color.h - Data structures and function prototypes for coloring algorithm
 *             to determine register allocation.
 */

#include "temp.h"
#include "graph.h"

struct COL_result {Temp_map coloring; Temp_tempList spills;};
struct COL_result COL_color(G_graph ig, Temp_map initial, Temp_tempList regs);


#endif // TIGER_COLOR_H_
