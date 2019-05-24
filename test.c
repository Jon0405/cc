#include <stdlib.h>
#include <stdio.h>

#include "cc.h"

int expect(int line, int expected, int actual) {
	if (expected == actual)
		return 0;
	fprintf(stderr, "%d: %d expected, but got %d\n", line, expected, actual);
	exit(1);
}

int runtest() {
	printf("OK\n");
	return 0;
}

