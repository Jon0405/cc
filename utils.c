#include <stdlib.h>

#include "cc.h"

// var
Variable *new_var(int place, Type *type) {
	Variable *var = malloc(sizeof(Variable));
	var->place = place;
	var->type = type;
	return var;
}

// type
Type *new_type(int ty, Type *ptrof) {
	Type *type = malloc(sizeof(Type));
	type->ty = ty;
	type->ptrof = ptrof;
	type->array_size = 0;
	return type; 
}

// node
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

Node *new_node_funcdef(char *name) {
	Node *node = malloc(sizeof(Node));
	node->ty = ND_DEF;
	node->name = name;
	node->argv = new_vlist();
	return node;
}
Node *new_node_block(Vlist *stmts) {
	Node *node = malloc(sizeof(Node));
	node->ty = ND_BLOCK;
	node->stmts = stmts;
	return node;
}
