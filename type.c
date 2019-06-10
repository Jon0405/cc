#include <stdlib.h>

#include "cc.h"

Type *new_type(int ty, Type *ptrof) {
	Type *type = malloc(sizeof(Type));
	type->ty = ty;
	type->ptrof = ptrof;
	return type; 
}
