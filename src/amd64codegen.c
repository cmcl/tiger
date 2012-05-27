#include "codegen.h"
#include "temp.h"

static AS_instrList instrList = NULL, last = NULL;

static void emit(AS_instr instr);
static void munchStm(T_stm stm);
static Temp_temp munchExp(T_exp expr);

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
				// cases for memory stm
			else if (dst->kind == T_TEMP)
				emit(AS_Move(String(), Temp_TempList(dst, NULL),
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
