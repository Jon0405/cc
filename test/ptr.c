#include <stdlib.h>

int *arr(int size) {
	int i;
	int *arr = malloc(size * sizeof(int));
	for (i = 0; i < size; i++) {
		if (i < 2)
			arr[i] = i;
		else
			arr[i] = arr[i-1] + arr[i-2];
	}
	return arr;
}
