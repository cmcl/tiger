/*
 * util.c - commonly used utility functions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "util.h"

#define BUFSIZE 1024

void *checked_malloc(int len)
{
	void *p = malloc(len);
	if (!p) {
		fprintf(stderr, "\nRan out of memory!\n");
		exit(1);
	}
	return p;
}

void *checked_realloc(void *p, size_t size)
{
	void *ptr = realloc(p, size);
	if (!ptr) {
		fprintf(stderr, "\nRand out of memory (realloc)!\n");
		exit(1);
	}
	return ptr;
}

string String(char *s)
{
	string p = checked_malloc(strlen(s) + 1);
	strcpy(p, s);
	return p;
}

string String_format(const char *s, ...)
{
	char buffer[BUFSIZE], *result = NULL;
	const char *p = s; /* cursor pointer */
	const char *str = NULL; /* pointer to variable argument strings */
	int len = 0; /* size of result */
	int i = 0; /* size of buffer */
	int n = 0; /* length of variable argument strings */
	bool isDigit = FALSE; /* needed so we can free memory allocated to string */
	va_list ap;
	va_start(ap, s);
	for (; *p; p++) {
		if (*p == '%') {
			switch (*++p) {
			case 's':
					str = va_arg(ap, const char *);
					break;
			case 'd':
					str = String_from_int(va_arg(ap, int));
					isDigit = TRUE;
					break;
			default:
					assert(0); /* Invalid format specifier */
			}
			n = strlen(str);
		} else {
			if (i < BUFSIZE - 1) {
				buffer[i++] = *p; continue;
			} else {
				str = p;
				n = 1;
			}
		}
		if (i + n > BUFSIZE) {
			result = checked_realloc(result, sizeof(*result) * (len + i + 1));
			if (len > 0) strncat(result, buffer, i);
			else strncpy(result, buffer, i);
			len += i;
			i = 0;
		}
		strncpy(buffer + i, str, n);
		i += n;
		if (isDigit) { free((void *)str); str = NULL; isDigit = FALSE; }
	}
	if (i > 0) {
		result = checked_realloc(result, sizeof(*result) * (len + i + 1));
		if (len > 0) strncat(result, buffer, i);
		else strncpy(result, buffer, i);
		/* can forget about i and len here, since we are exiting */
	}
	va_end(ap);
	return result;
}

string String_from_int(int n)
{
	char *str = checked_malloc(sizeof(*str) * (BUFSIZE + 1));
	snprintf(str, BUFSIZE + 1, "%d", n);
	return str;
}

U_boolList U_BoolList(bool head, U_boolList tail)
{
	U_boolList list = checked_malloc(sizeof(*list));
	list->head = head;
	list->tail = tail;
	return list;
}
