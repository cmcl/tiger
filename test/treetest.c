/*
 * main.c
 */

#include <stdio.h>
#include "util.h"
#include "symbol.h"
#include "absyn.h"
#include "errormsg.h"
#include "temp.h" /* needed by translate.h */
#include "tree.h" /* needed by frame.h */
#include "frame.h" /* needed by translate.h and printfrags prototype */
#include "semant.h" /* function prototype for transProg */
#include "canon.h"
#include "prabsyn.h"
#include "printtree.h"
#include "escape.h"
#include "parse.h"

extern bool anyErrors;

/* print the assembly language instructions to filename.s */
static void doProc(FILE *out, F_frame frame, T_stm body)
{
	T_stmList stmList;
	
	stmList = C_linearize(body);
	stmList = C_traceSchedule(C_basicBlocks(stmList));
	printStmList(stdout, stmList);
}

int main(int argc, char *argv[])
{
	A_exp absyn_root;
	F_fragList frags;
	char outfile[100];
	FILE *out = NULL;
	
	if (argc == 2) {
		absyn_root = parse(argv[1]);
		if (!absyn_root)
			return 1;
		 
		#if 0
			pr_exp(out, absyn_root, 0); /* print absyn data structure */
			fprintf(out, "\n");
		#endif
		
		Esc_findEscape(absyn_root); /* set varDec's escape field */
		
		frags = SEM_transProg(absyn_root);
		if (anyErrors) return 1; /* don't continue */
		
		/* convert the filename */
		sprintf(outfile, "%s.s", argv[1]);
		/* Chapter 8 */
		for (;frags;frags=frags->tail)
			if (frags->head->kind == F_procFrag) 
				doProc(out, frags->head->u.proc.frame, frags->head->u.proc.body);
			else if (frags->head->kind == F_stringFrag) {
				if (out == NULL) {
					out = fopen(outfile, "w");
					if (out == NULL) assert(0 && "Cannot create file.");
				}
				fprintf(out, "%s\n", frags->head->u.stringg.str);
			}
	
		if (out) fclose(out);
		return 0;
	}
	EM_error(0,"usage: tiger file.tig");
	return 1;
}
