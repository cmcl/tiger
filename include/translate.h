#ifndef TIGER_TRANSLATE_H_
#define TIGER_TRANSLATE_H_

#include "absyn.h"
#include "util.h"
#include "temp.h"
#include "frame.h"

typedef struct Tr_exp_ *Tr_exp;
typedef struct Tr_expList_ *Tr_expList;

typedef struct patchList_ *patchList;

typedef struct Tr_level_ *Tr_level;
typedef struct Tr_access_ *Tr_access;
typedef struct Tr_accessList_ *Tr_accessList;

struct Tr_accessList_ {
	Tr_access head;
	Tr_accessList tail;
};

Tr_level Tr_outermost(void);
Tr_level Tr_newLevel(Tr_level parent, Temp_label name, U_boolList formals);
Tr_access Tr_allocLocal(Tr_level level, bool escape);
Tr_accessList Tr_formals(Tr_level level);
Tr_accessList Tr_AccessList(Tr_access head, Tr_accessList tail);

Tr_expList Tr_ExpList(void);
/*
 * Add the translation expression to the end of the
 * list.
 */
void Tr_ExpList_append(Tr_expList list, Tr_exp expr);

/*
 * Add the translation expression to the front of the
 * list.
 */
void Tr_ExpList_prepend(Tr_expList list, Tr_exp expr);

int Tr_ExpList_empty(Tr_expList list);

/*
 * Translates a sequence of expressions
 * This function requires the result, of executing the body
 * of the let expression, to be the first element.
 */
Tr_exp Tr_seqExp(Tr_expList list);

Tr_exp Tr_simpleVar(Tr_access access, Tr_level level);
Tr_exp Tr_fieldVar(Tr_exp recordBase, int fieldOffset);
Tr_exp Tr_subscriptVar(Tr_exp arrayBase, Tr_exp index);

Tr_exp Tr_arrayExp(Tr_exp size, Tr_exp init);
Tr_exp Tr_recordExp(int n, Tr_expList list);

/*
 * Takes an expression list representing the let expression.
 * This function requires the result, of executing the body
 * of the let expression, to be the first element.
 */
Tr_exp Tr_letExp(Tr_expList list);

/*
 * Returns some linking information to be presented to
 * the body of the loop while translating.
 */
Tr_exp Tr_doneExp(void); /* for while/for loops */

Tr_exp Tr_breakExp(Tr_exp breakk);
Tr_exp Tr_whileExp(Tr_exp test, Tr_exp done, Tr_exp body);
Tr_exp Tr_ifExp(Tr_exp test, Tr_exp then, Tr_exp elsee);
Tr_exp Tr_assignExp(Tr_exp var, Tr_exp exp);

/*
 * Function for + - * / operators
 */
Tr_exp Tr_arithExp(A_oper op, Tr_exp left, Tr_exp right);

/*
 * Function for the relation operators operating on integer types
 */
Tr_exp Tr_relExp(A_oper op, Tr_exp left, Tr_exp right);

/*
 * Function for the equality (and not-equal) operator operating on integer types.
 */
Tr_exp Tr_eqExp(A_oper op, Tr_exp left, Tr_exp right);

/*
 * To be used for strings on eq/neq only.
 */
Tr_exp Tr_eqStringExp(A_oper op, Tr_exp left, Tr_exp right);


/*
 * To be used to compare references e.g. array and record types
 */
Tr_exp Tr_eqRef(A_oper op, Tr_exp left, Tr_exp right);

Tr_exp Tr_callExp(Tr_level level, Tr_level funLevel, Temp_label funLabel, Tr_expList argList);
Tr_exp Tr_stringExp(string str);
Tr_exp Tr_intExp(int n);
Tr_exp Tr_nilExp(void); /* provisional */
Tr_exp Tr_noExp(void);

void Tr_procEntryExit(Tr_level level, Tr_exp body, Tr_accessList formals);
F_fragList Tr_getResult(void);

#endif /* TIGER_TRANSLATE_H_ */
