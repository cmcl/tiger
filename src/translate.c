/*
 * Translate module for translating abstract syntax tree
 * to IR form.
 * Created by Craig McL on 4/5/2012
 */
#include "translate.h"
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

static Tr_exp Tr_ifExpNoElse(Tr_exp test, Tr_exp then);
static Tr_exp Tr_ifExpWithElse(Tr_exp test, Tr_exp then, Tr_exp elsee);

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
	T_exp addr = T_Temp(F_FP());
	/* Follow static links until we reach level of defintion */
	while (level != access->level) {
		/* Static link is the first frame formal */
		F_access staticLink = F_formals(level->frame)->head;
		addr = F_Exp(staticLink, addr);
		level = level->parent;
	}
	return Tr_Ex(F_Exp(access->access, addr));
}

Tr_exp Tr_fieldVar(Tr_exp recordBase, int fieldOffset)
{
	return Tr_Ex(T_Mem(T_Binop(T_plus, unEx(recordBase), T_Const(fieldOffset * F_WORD_SIZE))));
}

Tr_exp Tr_subscriptVar(Tr_exp arrayBase, Tr_exp index)
{
	return Tr_Ex(T_Mem(T_Binop(T_plus, unEx(arrayBase), 
				T_Binop(T_mul, unEx(index), T_Const(F_WORD_SIZE)))));
}

Tr_exp Tr_arrayExp(Tr_exp size, Tr_exp init)
{
	return NULL;
}


Tr_exp Tr_doneExp(void)
{
	return Tr_Ex(T_Name(Temp_newlabel()));
}

Tr_exp Tr_breakExp(Tr_exp breakk)
{
	T_stm s = unNx(breakk);
	if (s->kind == T_LABEL)
		return Tr_Nx(T_Jump(T_Name(s->u.LABEL), Temp_LabelList(s->u.LABEL, NULL)));
	else assert(0);
}


Tr_exp Tr_whileExp(Tr_exp test, Tr_exp done, Tr_exp body)
{
	Temp_label testLabel = Temp_newlabel(), bodyLabel = Temp_newlabel();
	return Tr_Ex(T_Eseq(T_Jump(T_Name(testLabel), Temp_LabelList(testLabel, NULL)),
				T_Eseq(T_Label(bodyLabel), T_Eseq(unNx(body),
					T_Eseq(T_Label(testLabel),
						T_Eseq(T_Cjump(T_eq, unEx(test), T_Const(0), unEx(done)->u.NAME, bodyLabel),
								T_Eseq(T_Label(unEx(done)->u.NAME), T_Const(0))))))));
}

static Tr_exp Tr_ifExpNoElse(Tr_exp test, Tr_exp then)
{
	Temp_label t = Temp_newlabel(), f = Temp_newlabel();
	struct Cx cond = unCx(test);
	Tr_exp result = NULL;
	doPatch(cond.trues, t);
	doPatch(cond.falses, f);
	if (then->kind == Tr_nx) {
		result = T_Seq(cond.stm, T_Seq(T_Label(t), T_Seq(then->u.nx, T_Label(f)))); 
	} else if (then->kind == Tr_cx) {
		result = T_Seq(cond.stm, T_Seq(T_Label(t), T_Seq(then->u.cx.stm, T_Label(f)))); 
	}
	return result;
}

static Tr_exp Tr_ifExpWithElse(Tr_exp test, Tr_exp then, Tr_exp elsee)
{
	Temp_label t = Temp_newlabel(), f = Temp_newlabel(), join = Temp_newlabel();
	Temp_temp r = Temp_newtemp();
	Tr_exp result = NULL;
	T_stm joinJump = T_Jump(T_Label(join), Temp_LabelList(join, NULL));
	struct Cx cond = unCx(test);
	doPatch(cond.trues, t);
	doPatch(cond.falses, f);
	if (elsee->kind == Tr_ex) {
		result = T_Eseq(cond.stm, T_Eseq(T_Label(t), T_Eseq(T_Move(T_Temp(r), unEx(then)),
					T_Eseq(T_Jump(join, Temp_LabelList(join, NULL)), T_Eseq(T_Label(f),
							T_Eseq(T_Move(T_Temp(r), unEx(elsee)), 
								T_Eseq(joinJump, T_Eseq(T_Label(join), T_Temp(r)))))))));
	} else {
		T_stm thenStm = (then->kind == Tr_nx) ? then->u.nx : then->u.cx.stm;
		T_stm elseeStm = (elsee->kind == Tr_cx) ? elsee->u.nx : elsee->u.cx.stm;
		result = T_Seq(cond.stm, T_Seq(T_Label(t), T_Seq(thenStm,
					T_Seq(T_Jump(join, Temp_LabelList(join, NULL)), T_Seq(T_Label(f),
							T_Seq(elseeStm, T_Seq(joinJump, T_Label(join))))))));
	}
	return result;
}

Tr_exp Tr_ifExp(Tr_exp test, Tr_exp then, Tr_exp elsee)
{
	if (elsee) return Tr_ifExpWithElse(test, then, elsee);
	else return Tr_ifExpNoElse(test, then);
}

Tr_exp Tr_assignExp(Tr_exp var, Tr_exp exp)
{
	return Tr_Nx(T_Move(unEx(var), unEx(exp)));
}

Tr_exp Tr_arithExp(A_oper op, Tr_exp left, Tr_exp right)
{
	T_binOp oper;
	switch(op) {
		case A_plusOp: oper = T_plus; break;
		case A_minusOp: oper = T_minus; break;
		case A_timesOp: oper = T_mul; break;
		case A_divideOp: oper = T_div; break;
	}
	return Tr_Ex(T_Binop(oper, unEx(left), unEx(right)));
}

Tr_exp Tr_relExp(A_oper op, Tr_exp left, Tr_exp right)
{
	T_relOp oper;
	switch(op) {
		case A_ltOp: oper = T_lt; break;
		case A_leOp: oper = T_gt; break;
		case A_gtOp: oper = T_le; break;
		case A_geOp: oper = T_ge; break;
	}
	T_stm cond = T_Cjump(oper, unEx(left), unEx(right), NULL, NULL);
	patchList trues = PatchList(&cond->u.CJUMP.true, NULL);
	patchList falses = PatchList(&cond->u.CJUMP.false, NULL);
	return Tr_Cx(trues, falses, cond);
}

/*Tr_exp Tr_callExp(Temp_label funLabel, Tr_expList argList)
{
	return Tr_Ex(T_Call(T_Name(funLabel), T_expList));
}*/

static F_fragList stringFragList = NULL;
Tr_exp Tr_stringExp(string str)
{
	Temp_label strLabel = Temp_newlabel();
	F_frag fragment = F_StringFrag(strLabel, str);
	stringFragList = F_FragList(fragment, stringFragList);
	return Tr_Ex(T_Name(strLabel));
}

Tr_exp Tr_intExp(int n)
{
	return Tr_Ex(T_Const(n));
}

Tr_exp Tr_nilExp(void)
{
	return Tr_Ex(T_Const(0));
}

Tr_exp Tr_noExp(void)
{
	return Tr_Ex(T_Const(0));
}
