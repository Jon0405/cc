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
	Vector *vec = new_vector();
	expect(__LINE__, 0, vec->len);

	for (int i = 0; i < 100; i++) {
		int *data = malloc(sizeof(int));
		*data = i;
		vec_push(vec, data);
	}

	expect(__LINE__, 100, vec->len);
	expect(__LINE__, 0, *((int *)vec->data[0]));
	expect(__LINE__, 50, *((int *)vec->data[50]));
	expect(__LINE__, 99, *((int *)vec->data[99]));

	printf("OK\n");
	return 0;
}

