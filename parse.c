#include <stdlib.h>

#include "cc.h"

extern Vlist *tokens;
extern Vlist *code;
extern Vlist *variables;
extern int vcount;

int consume(int ty) {
	if (((Token *)(tokens->data))->ty != ty)
		return 0;
	tokens = tokens->next;
	return 1;
}

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
	Node *node = NULL;

	if (consume(TK_RET)) {
		node = malloc(sizeof(Node));
		node->ty = ND_RET;
		node->lhs = expr();
	} else if (consume(TK_IF)) {
		if (!consume('('))
			error_at(((Token *)(tokens->data))->input, "should be '('!");
		node = expr();
		if (!consume(')'))
			error_at(((Token *)(tokens->data))->input, "should be ')'!");

		Node *rnode = stmt();
		if (consume(TK_ELSE))
			rnode = new_node(ND_ELSE, rnode, stmt());
		node = new_node(ND_IF, node, rnode);

		return node;
	} else if (consume(TK_WHILE)) {
		if (!consume('('))
			error_at(((Token *)(tokens->data))->input, "should be '('!");
		node = expr();
		if (!consume(')'))
			error_at(((Token *)(tokens->data))->input, "should be ')'!");

		node = new_node(ND_WHILE, node, stmt());

		return node;
	} else if (consume(TK_FOR)) {
		if (!consume('('))
			error_at(((Token *)(tokens->data))->input, "should be '('!");
		Node *initnode = expr();
		if (!consume(';'))
			error_at(((Token *)(tokens->data))->input, "should be ';'!");
		Node *condnode = expr();
		if (!consume(';'))
			error_at(((Token *)(tokens->data))->input, "should be ';'!");
		Node *increnode = expr();
		if (!consume(')'))
			error_at(((Token *)(tokens->data))->input, "should be ')'!");
		node = new_node(ND_FOR_INIT, initnode, new_node(ND_FOR_COND, condnode, increnode));

		node = new_node(ND_FOR, node, stmt());

		return node;
	} else if (consume('{')) {
		Vlist *stmts = new_vlist();
		vlist_push(stmts, stmt());
		while (!consume('}'))
			vlist_push(stmts, stmt());

		node = new_node_block(stmts);
			
		return node;
	} else {
		node = expr();
	}

	if (!consume(';'))
		error_at(((Token *)(tokens->data))->input, "should be ';'!");
	return node;
}

void program() {
	while (((Token *)(tokens->data))->ty != TK_EOF)
		vlist_push(code, stmt());
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
	Node *node = NULL;

	// if token is '(', it should be "'(' expr ')'"
	if (consume('(')) {
		node = expr();
		if (!consume(')'))
			error_at(((Token *)(tokens->data))->input, "should be ')'!");
		return node;
	}

	// otherwise, it should be a value or identifier
	if (((Token *)(tokens->data))->ty == TK_NUM) {
		node = new_node_num(((Token *)(tokens->data))->val);
		tokens = tokens->next;
		return node;
	}

	if (((Token *)(tokens->data))->ty == TK_IDENT) {
		char *ident_name = ((Token *)(tokens->data))->name;
		tokens = tokens->next;

		if (consume('(')) {
			node = new_node_call(ident_name);
			node->argv = new_vlist();
			for (;;) {
				Node *arg = expr();
				vlist_push(node->argv, arg);
				if (!consume(','))
					break;
			}
			if (!consume(')'))
				error_at(((Token *)(tokens->data))->input, "should be ')'!");
		} else {
			if (!map_get(variables, ident_name))
			{
				int *place = malloc(sizeof(int));
				*place = ++vcount;
				map_put(variables, ident_name, place);
			}
			node = new_node_ident(ident_name);
		}
		return node;
	}
	
	error_at(((Token *)(tokens->data))->input, "unexpected token");
	return node; // prevent compiler warning
}
