#include "semant.h"
#include "util.h"
#include "errormsg.h"
#include "env.h"

/* prototypes for functions local to this module */
static struct expty transExp(S_table venv, S_table tenv, A_exp a);
static struct expty transVar(S_table venv, S_table tenv, A_var v);
static void transDec(S_table venv, S_table tenv, A_dec d);
static Ty_ty transTy(S_table tenv, A_ty t);
static struct expty expTy(Tr_exp exp, Ty_ty ty);

/* expty constructor */
static struct expty expTy(Tr_exp exp, Ty_ty ty)
{
	struct expty e;
	e.exp = exp; e.ty = ty;
	return e;
}

static Ty_ty actual_ty(Ty_ty ty)
{
	if (ty->kind == Ty_name)
		return actual_ty(ty->u.name.ty);
	else
		return ty;
}


void SEM_transProg(A_exp exp)
{

}

static struct expty transExp(S_table venv, S_table tenv, A_exp a)
{
	switch (a->kind) {
		case A_varExp:
			return transVar(venv, tenv, a->u.var);
		case A_nilExp:
			return expTy(NULL, Ty_Nil());
		case A_intExp:
			return expTy(NULL, Ty_Int());
		case A_stringExp:
			return expTy(NULL, Ty_String());
		case A_callExp:
			A_expList args;
			Ty_tylist formals;
			E_enventry x = S_look(venv, a->u.call.func);
			if (x && x->kind == E_funEntry) {
				// check type of formals
				formals = x->u.fun.formals;
				for (args = a->u.call.args; args && formals; 
					args = args->tail, formals = formals->tail) {
					struct expty arg = transExp(venv, tenv, args->head);
					if (arg.ty->kind != formals->head->kind)
						EM_error(args->head->pos, "incorrect type %s; expected %s",
							Ty_ToString(arg.ty->kind), Ty_ToString(formals->head->kind));
				}
				return expTy(NULL, actual_ty(x->u.fun.result));
			} else {
				EM_error(a->pos, "undefined function %s", S_name(a->u.call.func));
				return expTy(NULL, Ty_Int());
			}
		case A_opExp:
			A_oper oper = a->u.op.oper;
			struct expty left = transExp(venv, tenv, a->u.op.left);
			struct expty right = transExp(venv, tenv, a->u.op.right);
			switch (oper) {
				case A_plusOp:
				case A_minusOp:
				case A_timesOp:
				case A_divideOp:
					if (left.ty->kind != Ty_int)
						EM_error(a->u.op.left->pos, "integer required");
					if (right.ty->kind != Ty_int)
						EM_error(a->u.op.right->pos, "integer required");
					return expTy(NULL, Ty_Int());
			}
			if (oper == A_eqOp)
			{

			}
			else if (oper == A_neqOp)
			{

			}
			else if (oper == A_ltOp)
			{

			}
			else if (oper == A_leOp)
			{

			}
			else if (oper == A_gtOp)
			{

			}
			else if (oper == A_geOp)
			{

			}
			return expTy(NULL, Ty_Int());
		case A_recordExp:
			Ty_ty typ = S_look(tenv, a->u.record.sym);
			if (!typ)
				EM_error(a->u.record.typ->pos, "undefined type");

			return expTy(NULL, Ty_Record(NULL));
		case A_seqExp:
			struct expty exp;
			A_expList seq;
			for (seq = a->u.seq; seq; seq = seq->tail)
				exp = transExp(venv, tenv, seq->head);
			return exp;
		case A_assignExp:
			struct expty var = transVar(venv, tenv, a->u.assign.var);
			struct expty exp = transExp(venv, tenv, a->u.assign.exp);
			if (exp.ty->kind != var.ty->kind)
				EM_error(a->u.assign.exp->pos, "expression not of expected type");
			return expTy(NULL, Ty_Void());
		case A_ifExp:
			struct expty test, then, elsee;
			test = transExp(venv, tenv, a->u.iff.test);
			if (test.ty->kind != Ty_int)
				EM_error(a->u.iff.test->pos, "integer required");
			then = transExp(venv, tenv, a->u.iff.then);
			if (a->u.iff.elsee)
				transExp(venv, tenv, a->u.iff.elsee);
			else if (then.ty->kind != Ty_void)
				EM_error(a->u.iff.then->pos, "must produce no value");
			return expTy(NULL, Ty_Void());
		case A_whileExp:
			struct expty test = transExp(venv, tenv, a->u.whilee.test);
			if (test.ty->kind != Ty_int)
				EM_error(a->u.whilee.test->pos, "integer required");
			struct expty body = transExp(venv, tenv, a->u.whilee.body);
			if (body.ty->kind != Ty_void)
				EM_error(a->u.whilee.body->pos, "must produce no value");
			return expTy(NULL, Ty_Void());
		case A_forExp:
			transExp(venv, tenv, a->u.forr.lo);
			transExp(venv, tenv, a->u.forr.hi);
			struct expty body = transExp(venv, tenv, a->u.forr.body);
			if (body.ty->kind != Ty_void)
				EM_error(a->u.forr.body->pos, "must produce no value");
			return expTy(NULL, Ty_Void());
		case A_breakExp:
			return expTy(NULL, Ty_Void());
		case A_letExp:
			struct expty exp;
			A_decList d;
			S_beginScope(venv);
			S_beginScope(tenv);
			for (d = a->u.let.decs; d; d = d->tail)
				transDec(venv, tenv, d->head);
			exp = transExp(venv, tenv, a->u.let.body);
			S_endScope(venv);
			S_endScope(tenv);
			return exp;
		case A_arrayExp:
			Ty_ty typ = S_look(tenv, a->u.array.typ);
			if (!typ)
				EM_error(a->u.array.typ->pos, "undefined type");
			struct expty size = transExp(venv, tenv, a->u.array.size);
			struct expty init = transExp(venv, tenv, a->u.array.init);
			if (size.ty->kind != Ty_int)
				EM_error(a->u.array.size->pos, "integer required");
			return expTy(NULL, Ty_Array(typ));
	}
}

static struct expty transVar(S_table venv, S_table tenv, A_var v)
{
	switch(v->kind) {
		case A_simpleVar:
			E_enventry x = S_look(venv, v->u.simple);
			if (x && x->kind == E_varEntry)
				return expTy(NULL, actual_ty(x->u.var.ty));
			else {
				EM_error(v->pos, "undefined variable %s", S_name(var->u.simple));
				return expTy(NULL, Ty_Int());
			}
		case A_fieldVar:
			return expTy(NULL, NULL/* Fill in*/);
		case A_subscriptVar:
			return transVar(venv, tenv, v->u.subscript.var);
	}
}


static void transDec(S_table venv, S_table tenv, A_dec d)
{
	switch (d->kind) {
		case A_functionDec:
			return;
		case A_varDec:
			return;
		case A_typeDec:
			return;
	}
	//assert(0);
}


static Ty_ty transTy(S_table tenv, A_ty t)
{
	switch (t->kind) {
		case A_nameTy:
			return Ty_Name(t->u.name, NULL);
		case A_recordTy:
			return Ty_Record(NULL);
		case A_arrayTy:
			return Ty_Array(NULL);
	}
	//assert(0);
}
