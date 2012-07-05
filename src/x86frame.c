/*
 * Implementation of frame interface for Intel x86 architecture
 * using Intel® 64 and IA-32 Architectures Software Developer’s Manual
 *
 * Created by Craig McL on 2/6/2012
 */
#include "frame.h"

struct F_access_ {
	enum { inFrame, inReg } kind;
	union {
		int offset; /* inFrame */
		Temp_temp reg; /* inReg */
	} u;
};

const int F_WORD_SIZE = 4; // Stack grows to lower address (64-bit machine - 8 bytes)
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

/* Make register functions should only be called once (inside one
 * of the temp list register generator functions).
 */
static Temp_tempList F_make_arg_regs(void);
static Temp_tempList F_make_calle_saves(void);
static Temp_tempList F_make_caller_saves(void);

static Temp_tempList F_special_registers(void);
static Temp_tempList F_arg_registers(void);
static Temp_tempList F_callee_saves(void);

static void F_add_to_map(string str, Temp_temp temp);

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
	int i = 0;
	for (fmls = formals; fmls; fmls = fmls->tail, i++) {
		F_access access = NULL;
		if (i < F_K && !fmls->head) {
			access = InReg(Temp_newtemp());
		} else {
			/* Keep a space for return address space. */
			access = InFrame((2 + i) * F_WORD_SIZE);
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

static Temp_tempList F_make_arg_regs(void)
{
	Temp_temp eax = Temp_newtemp(), ebx = Temp_newtemp(),
		ecx = Temp_newtemp(), edx = Temp_newtemp(), edi = Temp_newtemp(),
		esi = Temp_newtemp();
	F_add_to_map("eax", eax); F_add_to_map("ebx", ebx); F_add_to_map("ecx", ecx);
	F_add_to_map("edx", edx); F_add_to_map("edi", edi); F_add_to_map("esi", esi);
	return TL(eax, TL(ebx, TL(ecx, TL(edx, TL(edi, TL(esi, NULL))))));
}

static Temp_tempList F_make_calle_saves(void)
{
	Temp_temp ebx = Temp_newtemp();
	F_add_to_map("ebx", ebx);
	return TL(F_SP(), TL(F_FP(), TL(ebx, NULL)));
}

static Temp_tempList F_make_caller_saves(void)
{
	return TL(F_RV(), F_make_arg_regs());
}

static Temp_tempList F_special_registers(void)
{
	static Temp_tempList spregs = NULL;
	if (!spregs) {
		spregs = Temp_TempList(F_SP(), Temp_TempList(F_FP(),
			Temp_TempList(F_RV(), NULL)));
	}
	return spregs;
}

static Temp_tempList F_arg_registers(void)
{
	static Temp_tempList rarg = NULL;
	if (!rarg) {
		rarg = F_make_arg_regs();
	}
	return rarg;
}

static Temp_tempList F_callee_saves(void)
{
	static Temp_tempList callee_saves = NULL;
	if (!callee_saves) {
		callee_saves = F_make_calle_saves();
	}
	return callee_saves;
}

Temp_tempList F_caller_saves(void)
{
	static Temp_tempList caller_saves = NULL;
	if (!caller_saves) {
		caller_saves = F_make_caller_saves();
	}
	return caller_saves;
}

Temp_map F_tempMap = NULL;
static void F_add_to_map(string str, Temp_temp temp)
{
	if (!F_tempMap) {
		F_tempMap = Temp_name();
	}
	Temp_enter(F_tempMap, temp, str);
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

bool F_doesEscape(F_access access)
{
	return (access != NULL && access->kind == inFrame);
}

F_frag F_StringFrag(Temp_label label, string str)
{
	F_frag strfrag = checked_malloc(sizeof(*strfrag));
	strfrag->kind = F_stringFrag;
	strfrag->u.stringg.label = label;
	strfrag->u.stringg.str = str;
	return strfrag;
}

F_frag F_ProcFrag(T_stm body, F_frame frame)
{
	F_frag pfrag = checked_malloc(sizeof(*pfrag));
	pfrag->kind = F_procFrag;
	pfrag->u.proc.body = body;
	pfrag->u.proc.frame = frame;
	return pfrag;
}

F_fragList F_FragList(F_frag head, F_fragList tail)
{
	F_fragList fl = checked_malloc(sizeof(*fl));
	fl->head = head;
	fl->tail = tail;
	return fl;
}

Temp_tempList F_registers(void)
{
	return Temp_TempList_join(F_caller_saves(), F_callee_saves());
}

static Temp_temp fp = NULL;
Temp_temp F_FP(void)
{
	if (!fp) {
		fp = Temp_newtemp();
		F_add_to_map("ebp", fp);
	}
	return fp;
}

static Temp_temp sp = NULL;
Temp_temp F_SP(void)
{
	if (!sp) {
		sp = Temp_newtemp();
		F_add_to_map("esp", sp);
	}
	return sp;
}

static Temp_temp ra = NULL;
Temp_temp F_RA(void)
{
	if (!ra) {
		ra = Temp_newtemp();
		F_add_to_map("rdkd", ra);
	}
	return ra;
}

static Temp_temp rv = NULL;
Temp_temp F_RV(void)
{
	if (!rv) {
		rv = Temp_newtemp();
		F_add_to_map("eax", rv);
	}
	return rv;
}

T_exp F_Exp(F_access access, T_exp framePtr)
{
	if (access->kind == inFrame) {
		return T_Mem(T_Binop(T_plus, framePtr, T_Const(access->u.offset)));
	} else {
		return T_Temp(access->u.reg);
	}
}

T_exp F_externalCall(string str, T_expList args)
{
	return T_Call(T_Name(Temp_namedlabel(str)), args);
}

T_stm F_procEntryExit1(F_frame frame, T_stm stm)
{
	return stm; // dummy implementation
}

static Temp_tempList returnSink = NULL;
AS_instrList F_procEntryExit2(AS_instrList body)
{
	if (!returnSink) {
		returnSink = TL(F_RA(), F_callee_saves());
	}
	return AS_splice(body, 
		AS_InstrList(AS_Oper("", NULL, returnSink, NULL), NULL));
}

AS_proc F_procEntryExit3(F_frame frame, AS_instrList body)
{
	return AS_Proc("prolog", body, "epilog"); // dummy implementation
}

