#include <stdlib.h>

#include "cc.h"

// node operations
Node *new_node(int ty, Node *lhs, Node *rhs) {
	Node *node = malloc(sizeof(Node));
	node->ty = ty;
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

Node *new_node_num(int val) {
	Node *node = malloc(sizeof(Node));
	node->ty = ND_NUM;
	node->val = val;
	return node;
}

Node *new_node_ident(char *name) {
	Node *node = malloc(sizeof(Node));
	node->ty = ND_IDENT;
	node->name = name;
	return node;
}

Node *new_node_call(char *name) {
	Node *node = malloc(sizeof(Node));
	node->ty = ND_CALL;
	node->name = name;
	return node;
}

Node *new_node_block(Vlist *stmts) {
	Node *node = malloc(sizeof(Node));
	node->ty = ND_BLOCK;
	node->stmts = stmts;
	return node;
}
