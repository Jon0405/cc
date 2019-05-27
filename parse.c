#include <stdlib.h>
#include "cc.h"

extern Vlist *tokens;

int consume(int ty) {
	if (((Token *)(tokens->data))->ty != ty)
		return 0;
	tokens = tokens->next;
	return 1;
}

extern Node *code[100];

// parsing
Node *assign() {
	Node *node = equality();
	if (consume('='))
		node = new_node('=', node, assign());
	return node;
}

Node *expr() {
	return assign();
}

Node *stmt() {
	Node *node = expr();
	if (!consume(';'))
		error_at(((Token *)(tokens->data))->input, "this token is not ';'!");
	return node;
}

void program() {
	int i = 0;
	while (((Token *)(tokens->data))->ty != TK_EOF)
		code[i++] = stmt();
	code[i] = NULL;
}

Node *equality() {
	Node *node = relational();

	for (;;) {
		if (consume(TK_EQ))
			node = new_node(ND_EQ, node, relational());
		else if (consume(TK_NEQ))
			node = new_node(ND_NEQ, node, relational());
		else
			return node;
	}
}

Node *relational() {
	Node *node = add();

	for (;;) {
		if (consume(TK_LT))
			node = new_node(ND_LT, node, add());
		else if (consume(TK_LE))
			node = new_node(ND_LE, node, add());
		else if (consume(TK_BT))
			node = new_node(ND_LT, add(), node);
		else if (consume(TK_BE))
			node = new_node(ND_LE, add(), node);
		else
			return node;
	}
}

Node *add() {
	Node *node = mul();

	for (;;) {
		if (consume('+'))
			node = new_node('+', node, mul());
		else if (consume('-'))
			node = new_node('-', node, mul());
		else
			return node;
	}
}

Node *mul() {
	Node *node = unary();

	for (;;) {
		if (consume('*'))
			node = new_node('*', node, unary());
		else if (consume('/'))
			node = new_node('/', node, unary());
		else
			return node;
	}
}

Node *unary() {
	if (consume('+'))
		return term();
	if (consume('-'))
		return new_node('-', new_node_num(0), term());
	return term();
}

Node *term() {
	// if token is '(', it should be "'(' expr ')'"
	if (consume('(')) {
		Node *node = expr();
		if (!consume(')'))
			error_at(((Token *)(tokens->data))->input, "should be ')'!");
		return node;
	}

	// otherwise, it should be a value or identifier
	if (((Token *)(tokens->data))->ty == TK_NUM) {
		Node *node = new_node_num(((Token *)(tokens->data))->val);
		tokens = tokens->next;
		return node;
	}

	if (((Token *)(tokens->data))->ty == TK_IDENT) {
		Node *node = new_node_ident(((Token *)(tokens->data))->val);
		tokens = tokens->next;
		return node;
	}
	
	error_at(((Token *)(tokens->data))->input, "unexpected token");
}
