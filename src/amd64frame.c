/*
 * Implementation of frame interface for AMD64 architecture
 * using AMD64 Application Binary Interface Draft Version 0.99.5
 * section 3.2 (which is included in the docs folder for reference)
 * courtesy of http://www.x86-64.org/ (website active as of 4/5/2012)
 *
 * Created by Craig McL on 4/5/2012
 */
#include "frame.h"

struct F_access_ {
	enum { inFrame, inReg } kind;
	union {
		int offset; /* inFrame */
		Temp_temp reg; /* inReg */
	} u;
};

struct F_frame_ {
	Temp_label name;
	F_accessList formals;
	int local_count;
	/* instructions required to implement the "view shift" needed */
};

static F_access InFrame(int offset);
static F_access InReg(Temp_temp reg);
static F_accessList F_AccessList(F_access head, F_accessList tail);
static F_accessList makeAccessList(U_boolList formals);

static F_accessList F_AccessList(F_access head, F_accessList tail)
{
	F_accessList list = checked_malloc(sizeof(*list));
	list->head = head;
	list->tail = tail;
	return list;
}

static F_accessList makeAccessList(F_frame f, U_boolList formals)
{
	U_boolList fmls;
	F_accessList headList = tailList = NULL;
	for (fmls = formals; fmls; fmls = fmls->tail) {
		/* Assume all variables escape for now */
		F_access access = F_allocLocal(f, TRUE);
		if (headList) {
			tailList->tail = F_AccessList(access, NULL);
			tailList = tailList->tail;
		} else {
			headList = F_AccessList(access, NULL);
			tailList = headList;
		}
	}
	return headList;
}

F_frame F_newFrame(Temp_label name, U_boolList formals)
{
	F_frame f = checked_malloc(sizeof(*f));
	f->name = name;
	f->formals = makeAccessList(f, formals);
	f->local_count = 0;
	return f;
}

Temp_label F_name(F_frame f)
{
	return f->name;
}

F_accessList F_formals(F_frame f)
{
	return f->formals;
}

F_access F_allocLocal(F_frame f, bool escape)
{
	f->local_count++;
	if (escape) return InFrame();
	return InReg();
}

F_access InFrame(int offset)
{
	F_access fa = checked_malloc(sizeof(*fa));
	fa->kind = inFrame;
	fa->offset = offset;
	return fa;
}

F_access InReg(Temp_temp reg)
{
	F_access fa = checked_malloc(sizeof(*fa));
	fa->kind = inReg;
	fa->reg = reg;
	return fa;
}
