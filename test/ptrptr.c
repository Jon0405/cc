#include <stdlib.h>

int **ptrarr(int size) {
	int **arr = malloc(size * sizeof(int *));
	int i;
	for (i = 0; i < size; i++) {
		arr[i] = malloc(sizeof(int));
		if (i < 2)
			*(arr[i]) = i;
		else
			*(arr[i]) = *(arr[i-1]) + *(arr[i-2]);
	}
	return arr;
}
