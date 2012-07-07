#include <stdlib.h>
#include "codegen.h"
#include "temp.h"
#include "util.h"

static AS_instrList instrList = NULL, last = NULL;

static void emit(AS_instr instr);
static void munchStm(T_stm stm);
static Temp_temp munchExp(T_exp expr);
static Temp_tempList munchArgs(unsigned int n, T_expList eList,
								F_accessList formals);

static F_frame CODEGEN_frame = NULL; // for munchArgs

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
						emit(AS_Move(String_format("mov [`s0 + %d],`s1\n", n),
							NULL, TL(munchExp(e1), TL(munchExp(e2), NULL))));
							
				} else if (dst->u.MEM->kind == T_BINOP &&
					dst->u.MEM->u.BINOP.op == T_plus &&
					dst->u.MEM->u.BINOP.left->kind == T_CONST) {
						/* MOVE(MEM(BINOP(PLUS, CONST(n), e1)), e2) */
						T_exp e1 = dst->u.MEM->u.BINOP.right, e2 = src;
						int n = dst->u.MEM->u.BINOP.left->u.CONST;
						emit(AS_Move(String_format("mov [`s0 + %d],`s1\n", n),
							NULL, TL(munchExp(e1), TL(munchExp(e2), NULL))));
				} else if (dst->u.MEM->kind == T_CONST) {
					/* MOVE(MEM(CONST(n)), e2) */
					T_exp e2 = src;
					int n = dst->u.MEM->u.CONST;
					emit(AS_Move(String_format("mov [`s0 + %d],`s1\n", n),
							NULL, TL(munchExp(e2), NULL)));
				} else if (src->kind == T_MEM) {
					/* MOVE(MEM(e1), MEM(e2)) */
					T_exp e1 = dst->u.MEM, e2 = src->u.MEM;
					emit(AS_Move(String_format("mov [`s0],`s1\n"),
							NULL, TL(munchExp(e1), TL(munchExp(e2), NULL))));
				} else {
					/* MOVE(MEM(e1), e2) */
					T_exp e1 = dst->u.MEM, e2 = src;
					emit(AS_Move(String_format("mov [`s0],`s1\n"),
							NULL, TL(munchExp(e1), TL(munchExp(e2), NULL))));
				}
			else if (dst->kind == T_TEMP)
				/* MOVE(TEMP(e1), src) */
				emit(AS_Move(String_format("mov `d0,`s0\n"),
					TL(munchExp(dst), NULL), TL(munchExp(src), NULL)));
			else assert(0); /* destination of move must be temp or memory location */
			break;
		}
		case T_SEQ:
		{
			/* SEQ(stm1, stm2) */
			munchStm(stm->u.SEQ.left); munchStm(stm->u.SEQ.right);
			break;
		}
		case T_LABEL:
		{
			emit(AS_Label(String_format("%s:\n", Temp_labelstring(stm->u.LABEL)),
				stm->u.LABEL));
			break;
		}
		case T_JUMP:
		{
			Temp_temp r = munchExp(stm->u.JUMP.exp);
			emit(AS_Oper(String_format("jmp `d0\n"), TL(r, NULL), NULL,
				AS_Targets(stm->u.JUMP.jumps)));
			break;
		}
		case T_CJUMP:
		{
			Temp_temp left = munchExp(stm->u.CJUMP.left),
				right = munchExp(stm->u.CJUMP.right);
			emit(AS_Oper("cmp `s0,`s1\n", NULL, 
				TL(left, TL(right, NULL)), NULL));
			/* No need to deal with CJUMP false label
			 * as canonical module has it follow CJUMP */
			char *instr = NULL;
			switch (stm->u.CJUMP.op) {
				case T_eq:
					instr = "je"; break;
				case T_ne:
					instr = "jne"; break;
				case T_lt:
					instr = "jl"; break;
				case T_gt:
					instr = "jg"; break;
				case T_le:
					instr = "jle"; break;
				case T_ge:
					instr = "jge"; break;
				default: assert(0);
			}
			emit(AS_Oper(String_format("%s `j0\n", instr), NULL, NULL,
				AS_Targets(Temp_LabelList(stm->u.CJUMP.true, NULL))));
			break;
		}
		case T_EXP:
		{
			munchExp(stm->u.EXP);
			break;
		}
		default: assert(0);
	}
}

static Temp_temp munchExp(T_exp expr)
{
	switch(expr->kind) {
		case T_BINOP:
		{
			char *op = NULL, *sign = NULL;
			switch (expr->u.BINOP.op) {
			case T_plus:
				op = "add"; sign = "+"; break;
			case T_minus:
				op = "sub"; sign = "-"; break;
			case T_mul:
				op = "mul"; sign = "*"; break;
			case T_div:
				op = "div"; sign = "/"; break;
			default:
				assert(0 && "Invalid operator");
			}
			if (1)
				if (expr->u.BINOP.left->kind == T_CONST) {
					/* BINOP(op, CONST(i), e2) */
					Temp_temp r = Temp_newtemp();
					T_exp e2 = expr->u.BINOP.right;
					int n = expr->u.BINOP.left->u.CONST;
					emit(AS_Oper(String_format("%s `d0,`s0%s%d\n", 
												op, sign, n),
						TL(r, NULL), TL(munchExp(e2), NULL), NULL));
					return r;
				} else if (expr->u.BINOP.right->kind == T_CONST) {
					/* BINOP(op, e2, CONST(i)) */
					Temp_temp r = Temp_newtemp();
					T_exp e2 = expr->u.BINOP.left;
					int n = expr->u.BINOP.right->u.CONST;
					emit(AS_Oper(String_format("%s `d0,`s0%s%d\n",
												op, sign, n),
						TL(r, NULL), TL(munchExp(e2), NULL), NULL));
					return r;
				} else {
					/* BINOP(op, e1, e2) */
					Temp_temp r = Temp_newtemp();
					T_exp e1 = expr->u.BINOP.left, e2 = expr->u.BINOP.right;
					emit(AS_Oper(String_format("%s `d0,`s0%s`s1\n", 
												op, sign), 
						TL(r, NULL),
						TL(munchExp(e1), TL(munchExp(e2), NULL)), NULL));
					return r;
				}
			else assert(0);
			return NULL;
		}
		case T_MEM:
		{
			T_exp loc = expr->u.MEM;
			if (loc->kind == T_BINOP && loc->u.BINOP.op == T_plus)
				if (loc->u.BINOP.left->kind == T_CONST) {
					/* MEM(BINOP(PLUS, CONST(i), e2)) */
					Temp_temp r = Temp_newtemp();
					T_exp e2 = loc->u.BINOP.right;
					int n = loc->u.BINOP.left->u.CONST;
					emit(AS_Move(String_format("mov `d0,[`s0+%d]\n", n),
						TL(r, NULL), TL(munchExp(e2), NULL)));
					return r;
				} else if (loc->u.BINOP.right->kind == T_CONST) {
					/* MEM(BINOP(PLUS, e2, CONST(i))) */
					Temp_temp r = Temp_newtemp();
					T_exp e2 = loc->u.BINOP.left;
					int n = loc->u.BINOP.right->u.CONST;
					emit(AS_Move(String_format("mov `d0,[`s0+%d]\n", n),
						TL(r, NULL), TL(munchExp(e2), NULL)));
					return r;
				} else assert(0);
			else if (loc->kind == T_CONST) {
				/* MEM(CONST(i)) */
				Temp_temp r = Temp_newtemp();
				int n = loc->u.CONST;
				emit(AS_Move(String_format("mov `d0,[%d]\n", n), TL(r, NULL),
					NULL));
				return r; 
			} else {
				/* MEM(e1) */
				Temp_temp r = Temp_newtemp();
				T_exp e1 = loc->u.MEM;
				emit(AS_Move(String_format("mov `d0,[`s0]\n"), TL(r, NULL),
					TL(munchExp(e1), NULL)));
				return r;
			}
		}
		case T_TEMP:
		{
			/* TEMP(t) */
			return expr->u.TEMP;
		}
		case T_ESEQ:
		{
			/* ESEQ(e1, e2) */
			munchStm(expr->u.ESEQ.stm);
			return munchExp(expr->u.ESEQ.exp);
		}
		case T_NAME:
		{
			/* NAME(n) */
			Temp_temp r = Temp_newtemp();
			Temp_enter(F_tempMap, r, Temp_labelstring(expr->u.NAME));
			return r;
		}
		case T_CONST:
		{
			/* CONST(i) */
			Temp_temp r = Temp_newtemp();
			emit(AS_Move(String_format("mov `d0,%d\n", expr->u.CONST),
				TL(r, NULL), NULL));
			return r;
		}
		case T_CALL:
		{
			/* CALL(fun, args) */
			Temp_temp r = munchExp(expr->u.CALL.fun);
			Temp_tempList list = munchArgs(0, expr->u.CALL.args,
											F_formals(CODEGEN_frame));
			emit(AS_Oper("call `s0\n", F_caller_saves(), TL(r, list), NULL));
			return r;
		}
		default: assert(0);
	}
}

static char *register_names[] = {"eax", "ebx", "ecx", "edx", "edi", "esi"};
static unsigned int reg_count = 0;
static Temp_tempList munchArgs(unsigned int n, T_expList eList,
								F_accessList formals)
{
	if (!CODEGEN_frame) assert(0); // should never be NULL
	
	if (!eList || !formals) return NULL;
	
	// need first argument to be pushed onto stack last
	Temp_tempList tlist = munchArgs(n + 1, eList->tail, formals->tail);
	Temp_temp e = munchExp(eList->head);
	/* use the frame here to determine whether we push
	 * or move into a register. */
	if (F_doesEscape(formals->head)) 
		emit(AS_Oper("push `s0\n", NULL, TL(e, NULL), NULL));
	else {
		emit(AS_Move(
			String_format("mov %s,`s0", register_names[reg_count++]), 
			NULL, TL(e, NULL)));
	}
	return TL(e, tlist);
}

/*
 * Implementation of the the codegen interface.
 */

AS_instrList F_codegen(F_frame frame, T_stmList stmList)
{
	AS_instrList asList = NULL;
	T_stmList sList = stmList;
	CODEGEN_frame = frame; // set for munchArgs
	for (; sList; sList = sList->tail)
		munchStm(sList->head);
	asList = instrList;
	instrList = last = NULL;
	return asList;
}
