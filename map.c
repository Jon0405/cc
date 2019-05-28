#include <stdlib.h>
#include <string.h>

#include "cc.h"

void map_put(Vlist *map, char *key, void *val) {
	Map *new = malloc(sizeof(Map));
	new->key = key;
	new->val = val;
	vlist_push(map, new);
}

void *map_get(Vlist *map, char *key) {
	Vlist *curr = map->next; // skip head

	for (;;) {
		if (curr == NULL)
			return NULL; // not found

		if (strcmp(((Map *)(curr->data))->key, key) == 0)
			return ((Map *)(curr->data))->val;

		curr = curr->next;
	}
}
