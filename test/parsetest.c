/*
 * parsetest.c - Parse test source file.
 */

#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "symbol.h"
#include "absyn.h"
#include "errormsg.h"
#include "parse.h"
#include "prabsyn.h"

int main(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "usage: a.out filename\n");
		exit(1);
	}
	if (parse(argv[1])) pr_exp(stdout, absyn_root, 2);
	return 0;
}
