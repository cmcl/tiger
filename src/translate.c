/*
 * Translate module for translating abstract syntax tree
 * to IR form.
 * Created by Craig McL on 4/5/2012
 */
#include "translate.h"
#include "frame.h"
#include "tree.h"

struct Tr_level_ {
	Tr_level parent;
	Temp_label name;
	F_frame frame;
	Tr_accessList formals;
};

struct Tr_access_ {
	Tr_level level;
	F_access access;
};

struct Cx {
	patchList trues;
	patchList falses;
	T_stm stm;
};

struct Tr_exp_ {
	enum {Tr_ex, Tr_nx, Tr_cx} kind;
	union {
		T_exp ex;
		T_stm nx;
		struct Cx cx;
	} u;
};

struct patchList_ {
	Temp_label *head;
	patchList tail;
};

static Tr_accessList makeFormalAccessList(Tr_level level);
static Tr_access Tr_Access(Tr_level level, F_access access);

static Tr_exp Tr_Ex(T_exp ex);
static Tr_exp Tr_Nx(T_stm nx);
static Tr_exp Tr_Cx(patchList trues, patchList falses, T_stm stm);

static T_exp unEx(Tr_exp e);
static T_stm unNx(Tr_exp e);
static struct Cx unCx(Tr_exp e);

static patchList PatchList(Temp_label *head, patchList tail);
static void doPatch(patchList pList, Temp_label label);
static patchList joinPatch(patchList fList, patchList sList);

static Tr_level outer = NULL;
Tr_level Tr_outermost(void)
{
	if (!outer)
		outer = Tr_newLevel(NULL, Temp_newlabel(), NULL);
	return outer;
}

static Tr_accessList makeFormalAccessList(Tr_level level)
{
	Tr_accessList headList = NULL, tailList = NULL;
	/* Get the access list from the frame minus the first one (the static link)
	 * since it is not wanted from the level formals */
	F_accessList accessList = F_formals(level->frame)->tail;
	for (; accessList; accessList = accessList->tail) {
		Tr_access access = Tr_Access(level, accessList->head);
		if (headList) {
			tailList->tail = Tr_AccessList(access, NULL);
			tailList = tailList->tail;
		} else {
			headList = Tr_AccessList(access, NULL);
			tailList = headList;
		}
	}
	return headList;
}

static Tr_exp Tr_Ex(T_exp ex)
{
	Tr_exp trEx = checked_malloc(sizeof(*trEx));
	trEx->kind = Tr_ex;
	trEx->u.ex = ex;
	return trEx;
}

static Tr_exp Tr_Nx(T_stm nx)
{
	Tr_exp trNx = checked_malloc(sizeof(*trNx));
	trNx->kind = Tr_nx;
	trNx->u.nx = nx;
	return trNx;
}

static Tr_exp Tr_Cx(patchList trues, patchList falses, T_stm stm)
{
	Tr_exp trCx = checked_malloc(sizeof(*trCx));
	trCx->kind = Tr_cx;
	trCx->u.cx.trues = trues;
	trCx->u.cx.falses = falses;
	trCx->u.cx.stm = stm;
	return trCx;
}

static void doPatch(patchList pList, Temp_label label)
{
	for (; pList; pList = pList->tail)
		*(pList->head) = label;
}

static patchList joinPatch(patchList fList, patchList sList)
{
	if (!fList) return sList;
	for (; fList->tail; fList = fList->tail)
		;
	fList->tail = sList;
	return fList;
}

static T_exp unEx(Tr_exp e)
{
	switch(e->kind) {
		case Tr_ex:
			return e->u.ex;
		case Tr_nx:
			return T_Eseq(e->u.nx, T_Const(0));
		case Tr_cx:
		{
			Temp_temp r = Temp_newtemp();
			Temp_label t = Temp_newlabel(), f = Temp_newlabel();
			doPatch(e->u.cx.trues, t);
			doPatch(e->u.cx.falses, f);
			return T_Eseq(T_Move(T_Temp(r), T_Const(1)),
				T_Eseq(e->u.cx.stm,
					T_Eseq(T_Label(f), 
						T_Eseq(T_Move(T_Temp(r), T_Const(0)),
							T_Eseq(T_Label(t), T_Temp(r))))));
		}
		default:
		{
			assert(0);
		}
	}
	return NULL;
}

static T_stm unNx(Tr_exp e)
{
	switch(e->kind) {
		case Tr_ex:
			return T_Exp(e->u.ex);
		case Tr_nx:
			return e->u.nx;
		case Tr_cx:
		{
			Temp_temp r = Temp_newtemp();
			Temp_label t = Temp_newlabel(), f = Temp_newlabel();
			doPatch(e->u.cx.trues, t);
			doPatch(e->u.cx.falses, f);
			return T_Exp(T_Eseq(T_Move(T_Temp(r), T_Const(1)),
				T_Eseq(e->u.cx.stm,
					T_Eseq(T_Label(f), 
						T_Eseq(T_Move(T_Temp(r), T_Const(0)),
							T_Eseq(T_Label(t), T_Temp(r)))))));
		}
		default:
		{
			assert(0);
		}
	}
	return NULL;
}

static struct Cx unCx(Tr_exp e)
{
	switch(e->kind) {
		case Tr_ex:
		{
			struct Cx cx;
			/* If comparison yields true then the expression was false (compares equal
			 * to zero) so we jump to false label. */
			cx.stm = T_Cjump(T_eq, e->u.ex, T_Const(0), NULL, NULL);
			cx.trues = PatchList(&(cx.stm->u.CJUMP.false), NULL);
			cx.falses = PatchList(&(cx.stm->u.CJUMP.true), NULL);
			return cx;
		}
		case Tr_nx:
		{
			assert(0); // Should never occur
		}
		case Tr_cx:
		{
			return e->u.cx;
		}
		default:
		{
			assert(0);
		}
	}

}

static patchList PatchList(Temp_label *head, patchList tail)
{
	patchList pList = checked_malloc(sizeof(*pList));
	pList->head = head;
	pList->tail = tail;
	return pList;
}

Tr_access Tr_Access(Tr_level level, F_access access)
{
	Tr_access trAccess = checked_malloc(sizeof(*trAccess));
	trAccess->level = level;
	trAccess->access = access;
	return trAccess;
}

/*
 * Adds an extra formal parameter onto the list passed to
 * the frame constructor. This parameter represents the static link
 */
Tr_level Tr_newLevel(Tr_level parent, Temp_label name, U_boolList formals)
{
	Tr_level level = checked_malloc(sizeof(*level));
	level->parent = parent;
	level->name = name;
	level->frame = F_newFrame(name, U_BoolList(TRUE, formals));
	level->formals = makeFormalAccessList(level);
	return level;
}

Tr_access Tr_allocLocal(Tr_level level, bool escape)
{
	Tr_access local = checked_malloc(sizeof(*local));
	local->level = level;
	local->access = F_allocLocal(level->frame, escape);
	return local;
}

Tr_accessList Tr_formals(Tr_level level)
{
	return level->formals;
}

Tr_accessList Tr_AccessList(Tr_access head, Tr_accessList tail)
{
	Tr_accessList list = checked_malloc(sizeof(*list));
	list->head = head;
	list->tail = tail;
	return list;
}

Tr_exp Tr_simpleVar(Tr_access access, Tr_level level)
{
	/* TODO use static links to get to level where
	 * variable is declared */
	return Tr_Ex(F_Exp(access->access, T_Temp(F_FP()))); // not complete!
}
