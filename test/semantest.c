/*
 * Created by Craig McL on 20/4/2012
 * Parses a source file and performs semantic analysis
 * on the resulting root expression. This will produce
 * any errors encountered during the tree walk.
 */

#include "semant.h"
#include "util.h"
#include "symbol.h"
#include "absyn.h"
#include "errormsg.h"
#include "parse.h"
#include <stdio.h>
#include <stdlib.h>

extern int yyparse(void);

/* parse source file fname; 
   return abstract syntax data structure */
A_exp parse(string fname)
{
	EM_reset(fname);
	if (yyparse() == 0)	/* parsing worked */
		return absyn_root;
	else
		return NULL;
}

int main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "usage: semantest filename\n");
		exit(1);
	}
	if (parse(argv[1])) SEM_transProg(absyn_root);
	return 0;
}
