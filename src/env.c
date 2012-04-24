#include "util.h"
#include "env.h"
#include <stdlib.h>

E_enventry E_VarEntry(Ty_ty ty)
{
	E_enventry varEntry = checked_malloc(sizeof(*varEntry));
	varEntry->kind = E_varEntry;
	varEntry->u.var.ty = ty;
	return varEntry;
}

E_enventry E_FunEntry(Ty_tyList formals, Ty_ty result)
{
	E_enventry funEntry = checked_malloc(sizeof(*funEntry));
	funEntry->kind = E_funEntry;
	funEntry->u.fun.formals = formals;
	funEntry->u.fun.result = result;
	return funEntry;
}

S_table E_base_tenv(void)
{
	S_table tenv = S_empty();
	S_enter(tenv, S_Symbol("int"), Ty_Int());
	S_enter(tenv, S_Symbol("string"), Ty_String());
	return tenv;
}

S_table E_base_venv(void)
{
	S_table venv = S_empty();
	E_enventry print = E_FunEntry(Ty_TyList(Ty_String(), NULL), Ty_Void());
	S_enter(venv, S_Symbol("print"), print);
	/* TODO: Add some function definitions here */
	return venv;
}
