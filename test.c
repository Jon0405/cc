#include <stdlib.h>
#include <stdio.h>

#include "cc.h"

int expect(int line, int expected, int actual) {
	if (expected == actual)
		return 0;
	fprintf(stderr, "%d: %d expected, but got %d\n", line, expected, actual);
	exit(1);
}

void test_map() {
	char *testkey = "key";
	char *testval = "val";

	Vlist *map = new_vlist();

	expect(__LINE__, 0, (long)map_get(map, testkey));

	map_put(map, testkey, testval);
	expect(__LINE__, 0, (long)map_get(map, "fakekey"));
	expect(__LINE__, (long)testval, (long)map_get(map, testkey));
}

int runtest() {
	test_map();

	printf("OK\n");
	return 0;
}

