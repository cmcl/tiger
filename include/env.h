#ifndef _ENV_H_
#define _ENV_H_

#include "types.h"
#include "translate.h"
#include "symbol.h"

typedef struct E_enventry_ *E_enventry;

struct E_enventry_ {
	enum { E_varEntry, E_funEntry } kind;
	union {
		struct {
			Tr_access access;
			Ty_ty ty;
		} var;
		struct {
			Tr_level level;
			Tr_label label;
			Ty_tyList formals;
			Ty_ty result;
		} fun;
	} u;
};

E_enventry E_VarEntry(Ty_ty ty);
E_enventry E_FunEntry(Ty_tyList formals, Ty_ty result);

S_table E_base_tenv(void);	/* Ty_ty environment */
S_table E_base_venv(void);	/* E_ enventry environment */

#endif /* _ENV_H_ */
