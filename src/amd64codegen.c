#include "codegen.h"
#include "temp.h"

static AS_instrList instrList = NULL, last = NULL;

static void emit(AS_instr instr);
static void munchStm(T_stm stm);
static Temp_temp munchExp(T_exp expr);
static Temp_tempList TL(Temp_temp t, Temp_tempList l);

static Temp_tempList TL(Temp_temp t, Temp_tempList l)
{
	return Temp_TempList(t, l);
}

static void emit(AS_instr instr)
{
	if (!instrList) instrList = last = AS_InstrList(instr, NULL);
	else last = last->tail = AS_InstrList(instr, NULL);
}

static void munchStm(T_stm stm)
{
	switch(stm->kind) {
		case T_MOVE:
		{
			T_exp dst = stm->u.MOVE.dst, src = stm->u.MOVE.src;
			if (dst->kind == T_MEM)
				if (dst->u.MEM->kind == T_BINOP &&
					dst->u.MEM->u.BINOP.op == T_plus &&
					dst->u.MEM->u.BINOP.right->kind == T_CONST) {
						/* MOVE(MEM(BINOP(PLUS, e1, CONST(n))), e2) */
						T_exp e1 = dst->u.MEM->u.BINOP.left, e2 = src;
						int n = dst->u.MEM->u.BINOP.right->u.CONST;
						emit(AS_oper(String_format("mov [`s0 + %d],`s1\n", n),
							NULL, TL(munchExp(e1), TL(munchExp(e2), NULL)), NULL));
							
				} else if (dst->u.MEM->kind == T_BINOP &&
					dst->u.MEM->u.BINOP.op == T_plus &&
					dst->u.MEM->u.BINOP.left->kind == T_CONST) {
						/* MOVE(MEM(BINOP(PLUS, CONST(n), e1)), e2) */
						T_exp e1 = dst->u.MEM->u.BINOP.right, e2 = src;
						int n = dst->u.MEM->u.BINOP.left->u.CONST;
						emit(AS_oper(String_format("mov [`s0 + %d],`s1\n", n),
							NULL, TL(munchExp(e1), TL(munchExp(e2), NULL)), NULL));
				} else if (dst->u.MEM->kind == T_CONST) {
					/* MOVE(MEM(CONST(n)), e2) */
					T_exp e2 = src;
					int n = dst->u.MEM->u.CONST;
					emit(AS_oper(String_format("mov [`s0 + %d],`s1\n", n),
							NULL, TL(munchExp(e2), NULL), NULL));
				} else if (src->kind == T_MEM) {
					/* MOVE(MEM(e1), MEM(e2)) */
					T_exp e1 = dst->u.MEM, e2 = src->u.MEM;
					emit(AS_oper(String_format("mov [`s0],`s1\n"),
							NULL, TL(munchExp(e1), TL(munchExp(e2), NULL)), NULL));
				} else {
					/* MOVE(MEM(e1), e2) */
					T_exp e1 = dst->u.MEM, e2 = src;
					emit(AS_oper(String_format("mov [`s0],`s1\n"),
							NULL, TL(munchExp(e1), TL(munchExp(e2), NULL)), NULL));
				}
			else if (dst->kind == T_TEMP)
				emit(AS_Move(String_format("mov `d0,`s0\n"), Temp_TempList(dst, NULL),
						Temp_TempList(munchExp(src), NULL)));
			else assert(0); /* destination of move must be temp or memory location */
		}
		case T_SEQ:
		case T_LABEL:
		case T_JUMP:
		case T_CJUMP:
		case T_EXP:
		default: assert(0);
	}
}

static Temp_temp munchExp(T_exp expr)
{
	switch(expr->kind) {
		case T_BINOP:
		case T_MEM:
		case T_TEMP:
		case T_ESEQ:
		case T_NAME:
		case T_CONST:
		case T_CALL:
		default: assert(0);
	}
}

/*
 * Implementation of the the codegen interface.
 */

AS_instrList F_codegen(F_frame frame, T_stmList stmList)
{
	AS_instrList asList = NULL;
	T_stmList sList = stmList;
	for (; sList; sList = sList->tail)
		munchStm(sList->head);
	asList = instrList;
	instrList = last = NULL;
	return asList;
}
