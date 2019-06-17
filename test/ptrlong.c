#include <stdlib.h>

long *arr(int size) {
	int i;
	long *arr = malloc(size * sizeof(long));
	for (i = 0; i < size; i++) {
		if (i < 2)
			arr[i] = i;
		else
			arr[i] = arr[i-1] + arr[i-2];
	}
	return arr;
}
