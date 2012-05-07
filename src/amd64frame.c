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

const int F_WORD_SIZE = 8; // Stack grows to lower address (64-bit machine - 8 bytes)
static const int F_K = 6; // Number of parameters kept in registers
struct F_frame_ {
	Temp_label name;
	F_accessList formals;
	int local_count;
	/* instructions required to implement the "view shift" needed */
};

static F_access InFrame(int offset);
static F_access InReg(Temp_temp reg);
static F_accessList F_AccessList(F_access head, F_accessList tail);
static F_accessList makeFormalAccessList(F_frame f, U_boolList formals);

static F_accessList F_AccessList(F_access head, F_accessList tail)
{
	F_accessList list = checked_malloc(sizeof(*list));
	list->head = head;
	list->tail = tail;
	return list;
}

static F_accessList makeFormalAccessList(F_frame f, U_boolList formals)
{
	U_boolList fmls;
	F_accessList headList = NULL, tailList = NULL;
	int i = 1;
	for (fmls = formals; fmls; fmls = fmls->tail, i++) {
		F_access access = NULL;
		if (i <= F_K && !fmls->head) {
			access = InReg(Temp_newtemp());
		} else {
			/* Add 1 for return address space. */
			access = InFrame((1 + i) * F_WORD_SIZE);
		}
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

static F_access InFrame(int offset)
{
	F_access fa = checked_malloc(sizeof(*fa));
	fa->kind = inFrame;
	fa->u.offset = offset;
	return fa;
}

static F_access InReg(Temp_temp reg)
{
	F_access fa = checked_malloc(sizeof(*fa));
	fa->kind = inReg;
	fa->u.reg = reg;
	return fa;
}

F_frame F_newFrame(Temp_label name, U_boolList formals)
{
	F_frame f = checked_malloc(sizeof(*f));
	f->name = name;
	f->formals = makeFormalAccessList(f, formals);
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
	if (escape) return InFrame(F_WORD_SIZE * (- f->local_count));
	return InReg(Temp_newtemp());
}

static Temp_temp fp = NULL;
Temp_temp F_FP(void)
{
	if (!fp)
		fp = Temp_newtemp();
	return fp;
}

T_exp F_Exp(F_access access, T_exp framePtr)
{
	if (access->kind == inFrame) {
		return T_Mem(T_Binop(T_plus, framePtr, T_Const(access->u.offset)));
	} else {
		return T_Temp(access->u.reg);
	}
}


