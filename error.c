#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "cc.h"

extern char *user_input;

// print error messages like printf
void error(char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

// find error point
void error_at(char *loc, char *msg) {
	int err_pos = loc - user_input;
	fprintf(stderr, "%s\n", user_input);
	fprintf(stderr, "%*s", err_pos, ""); // output space
	fprintf(stderr, "^ %s\n", msg);
	exit(1);
}
