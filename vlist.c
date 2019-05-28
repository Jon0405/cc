#include <stdlib.h>

#include "cc.h"

Vlist *new_vlist() {
	Vlist *new_vlist = malloc(sizeof(Vlist));
	new_vlist->data = NULL;
	new_vlist->next = NULL;
	return new_vlist;
}

void vlist_push(Vlist *vlist, void *data) {
	Vlist *new_vlist = malloc(sizeof(Vlist));
	new_vlist->data = data;
	new_vlist->next = NULL;

	Vlist *curr = vlist;
	while (curr->next != NULL)
		curr = curr->next;
	curr->next = new_vlist;
}
