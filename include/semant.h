#ifndef _SEMANT_H_
#define _SEMANT_H_

#include "absyn.h"
#include "types.h"
#include "translate.h"

struct expty {Tr_exp exp; Ty_ty ty;};

void SEM_transProg(A_exp exp);

#endif /* _SEMANT_H_ */
