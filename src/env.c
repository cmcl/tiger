#include "util.h"
#include "env.h"
#include <stdlib.h>

S_table tenv = NULL;
S_table venv = NULL;

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
	funEntry->u.fun.formals = result;
	return funEntry;
}

S_table E_base_tenv(void)
{
	tenv = S_empty();
	S_enter(tenv, S_Symbol("int"), Ty_Int());
	S_enter(tenv, S_Symbol("string"), Ty_String());
	return tenv;
}

S_table E_base_venv(void)
{
	venv = S_empty();
	/* TODO: Add some function definitions here */
	return venv;
}
