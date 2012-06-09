#ifndef TIGER_REGALLOC_H_
#define TIGER_REGALLOC_H_

#include "frame.h"
#include "temp.h"
#include "assem.h"

/* function prototype from regalloc.c */
struct RA_result {Temp_map coloring; AS_instrList il;};
struct RA_result RA_regAlloc(F_frame f, AS_instrList il);

#endif // TIGER_REGALLOC_H_
