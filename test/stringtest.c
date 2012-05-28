/*
 * Test driver for String facilities
 */

#include "util.h"
#include <stdio.h>

int main(void)
{
	string s = String_from_int(524);
	puts(s);
	string str = String_format("Hello, World!\nHere is a string %s and an integer %d\n",
		"BOOM!", 2492);
	printf("%s", str);
	printf("%s", String_format("Hello, World!\n"));
	return 0;
}
