/* parsing tokens
stmt       = "return" expr ";"
           | expr ";"
           | "{" stmt* "}"
           | "if" "(" expr ")" stmt ("else" stmt)?
           | "while" "(" expr ")" stmt
           | "for" "(" expr? ";" expr? ";" expr? ")" stmt
expr       = assign
assign     = equality ("=" assign)?
equality   = relational ("==" relational | "!=" relational)*
relational = add ("<" add | "<=" add | ">" add | ">=" add)*
add        = mul ("+" mul | "-" mul)*
mul        = unary ("*" unary | "/" unary)*
unary      = "sizeof" unary
           | ("+" | "-" | "&")? term
           | "*"* unary
           | declare
declare    = ("int" | "long") ("*"*)? term
term       = num
           | ident ("(" (expr (",")?)*? ")")?
           | "(" expr ")"
funcdef    = ("int" | "long") ("*"?)? ident "(" (declare*)? ")"  
program    = funcdef stmt
*/

#include <stdlib.h>

#include "cc.h"

extern Vlist *tokens;
extern Vlist *variables;
extern Vlist *functions;
extern Vlist *return_type;
extern int *vcount;

int consume(int ty) {
	if (((Token *)(tokens->data))->ty != ty)
		return 0;
	tokens = tokens->next;
	return 1;
}

Type *consume_type() {
	Type *type = NULL;
	switch (((Token *)(tokens->data))->ty) {
		case TK_INT:
			type = new_type(INT, NULL);
			break;
		case TK_LONG:
			type = new_type(LONG, NULL);
			break;
		default:
			return NULL; // this token is not a type
	}

	tokens = tokens->next;

	while (consume('*'))
		type = new_type(PTR, type);

	return type;
}

Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *declare();
Node *term();

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
	if (consume(';'))
		return new_node(ND_NOP, NULL, NULL);

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

Node *ptr(Node *node) {
	Node *ptrnode = mul();

	if (node->ty == ND_IDENT) {
		Variable *var = map_get(variables, node->name);

		if (var->type->ty == PTR) {
			Type *next = var->type->ptrof;
			if (next->ty == INT)
				ptrnode = (new_node('*', ptrnode, new_node_num(4)));
			else
				ptrnode = (new_node('*', ptrnode, new_node_num(8)));
		}
	}

	return ptrnode;
}


Node *add() {
	Node *node = mul();

	for (;;) {
		if (consume('+'))
			node = new_node('+', node, ptr(node));
		else if (consume('-'))
			node = new_node('-', node, ptr(node));
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

Type *curr_ptrof; // trace ptrof

Node *unary() {
	if (consume('+'))
		return term();

	if (consume('-'))
		return new_node('-', new_node_num(0), term());

	if (consume('&')) {
		Node *node = term();
		if (node->ty != ND_IDENT && node->ty != ND_INDIR)
			error("get address from a non-variable value!");
		return new_node(ND_ADDR, node, NULL);
	}

	char *input = ((Token *)(tokens->data))->input;
	if (consume('*')) {
		Node *node = new_node(ND_INDIR, unary(), NULL);
		curr_ptrof = curr_ptrof->ptrof;
		if (curr_ptrof == NULL)
			error_at(input, "dereference a value!");
		return node;
	}

	if (consume(TK_SIZEOF)) {
		Node *node = unary();
		while (node->ty == '+' || node->ty == '-' || node->ty == '*' || node->ty == '/')
			node = node->lhs;

		if (node->ty == ND_NUM)
			return new_node_num(4);

		if (node->ty == ND_ADDR)
			return new_node_num(8);


		int deref = 0;
		for (; node->ty == ND_INDIR; node = node->lhs)
			deref++;
		if (node->ty == ND_IDENT) {
			Variable *var = map_get(variables, node->name);
			Type *type = var->type;
			for (; deref > 0; deref--)
				type = type->ptrof;
			switch (type->ty) {
				case INT:
					return new_node_num(4);
				case LONG:
					return new_node_num(8);
				case PTR:
					return new_node_num(8);
			}
		}
		error("sizeof unknown type!");
	}

	return declare();
}

Node *declare() {
	Type *type = consume_type();
	if (type != NULL) { // is a declaration
		if (((Token *)(tokens->data))->ty != TK_IDENT)
			error_at(((Token *)(tokens->data))->input, "should be an indentifier!");
		char *ident_name = ((Token *)(tokens->data))->name;
		switch (type->ty) {
			case INT:
				*vcount += 1;
				break;
			case LONG:
				*vcount += 2;
				break;
			case PTR:
				*vcount += 2;
		}
		Variable *var = new_var(*vcount, type);
		map_put(variables, ident_name, var);
	}

	return term();
}

Node *term() {
	Node *node = NULL;

	if (consume('(')) {
		node = expr();
		if (!consume(')'))
			error_at(((Token *)(tokens->data))->input, "should be ')'!");
	} else if (((Token *)(tokens->data))->ty == TK_NUM) {
		node = new_node_num(((Token *)(tokens->data))->val);
		tokens = tokens->next;
	} else if (((Token *)(tokens->data))->ty == TK_IDENT) {
		char *ident_name = ((Token *)(tokens->data))->name;
		tokens = tokens->next;

		if (consume('(')) {
			node = new_node_call(ident_name);
			node->argv = new_vlist();
			if (!consume(')')) {
				do {
					Node *arg = expr();
					vlist_push(node->argv, arg);
				} while (consume(','));
				if (!consume(')'))
					error_at(((Token *)(tokens->data))->input, "should be ')'!");
			}
		} else {
			Variable *var = map_get(variables, ident_name);
			if (var == NULL)
				error("undeclared variable!");
			curr_ptrof = var->type;
			node = new_node_ident(ident_name);
		}
	} else {
		error_at(((Token *)(tokens->data))->input, "unexpected token");
	}

	return node;
}

Node *funcdef() {
	Type *type = consume_type();
	if (!type)
		error_at(((Token *)(tokens->data))->input, "should be a function return type!");

	if (((Token *)(tokens->data))->ty != TK_IDENT)
		error_at(((Token *)(tokens->data))->input, "should be a function name!");

	char *ident_name = ((Token *)(tokens->data))->name;
	tokens = tokens->next;

	Node *node = new_node_funcdef(ident_name);
	if (!consume('('))
		error_at(((Token *)(tokens->data))->input, "should be '('!");

	if (!consume(')')) {
		do {
			Node *arg = declare();
			vlist_push(node->argv, arg);
		} while (consume(','));

		if (!consume(')'))
			error_at(((Token *)(tokens->data))->input, "should be ')'!");
	}
	map_put(return_type, ident_name, type);

	return node;
}

#define new_intptr(i) do { i = malloc(sizeof(int)); *i = 0; } while (0);

void program() {
	while (((Token *)(tokens->data))->ty != TK_EOF) {
		Func *func = malloc(sizeof(Func));
		new_intptr(func->vcount);
		vcount = func->vcount;
		variables = func->variables = new_vlist();

		Node *node = funcdef();
		func->name = node->name;
		func->nodedef = node;
		func->code = stmt();

		vlist_push(functions, func); // no gloabl variable available now
	}
}
