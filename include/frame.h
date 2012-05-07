#ifndef _FRAME_H_
#define _FRAME_H_

#include "temp.h"
#include "util.h"
#include "tree.h"

typedef struct F_frame_ *F_frame;
typedef struct F_access_ *F_access;

typedef struct F_accessList_ *F_accessList;

struct F_accessList_ {
	F_access head;
	F_accessList tail;
};

extern const int F_WORD_SIZE;

F_frame F_newFrame(Temp_label name, U_boolList formals);
Temp_label F_name(F_frame f);
F_accessList F_formals(F_frame f);
F_access F_allocLocal(F_frame f, bool escape);

Temp_temp F_FP(void);
T_exp F_Exp(F_access access, T_exp framePtr);

#endif /* _FRAME_H_ */
