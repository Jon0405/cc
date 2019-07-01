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

// consume tokens
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

	while (consume('*')) // dereference
		type = new_type(PTR, type);

	return type;
}

// parse nodes
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
Node *funcdef();

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

// TODO: deduplicate similar code
Node *ptr(Node *node) {
	Node *ptrnode = mul();

	if (node->ty == ND_IDENT) {
		Variable *var = map_get(variables, node->name);

		if (var->type->ty == PTR) {
			Type *next = var->type->ptrof;
			if (next->ty == INT){
				ptrnode = new_node('*', ptrnode, new_node_num(HALF_WORD));
			} else if (next->ty == LONG) {
				ptrnode = new_node('*', ptrnode, new_node_num(WORD));
			} else if (next->ty == PTR) {
				ptrnode = new_node('*', ptrnode, new_node_num(WORD));
			} else {
				error("unknown type!");
			}
		} else if (var->type->array_size != 0) {
			if (var->type->ty == INT){
				ptrnode = new_node('*', ptrnode, new_node_num(HALF_WORD));
			} else if (var->type->ty == LONG) {
				ptrnode = new_node('*', ptrnode, new_node_num(WORD));
			} else if (var->type->ty == PTR) {
				ptrnode = new_node('*', ptrnode, new_node_num(WORD));
			} else {
				error("unknown type!");
			}
		}
	} else if (node->ty == ND_ADDR) {
		Node *lnode = node->lhs;
		Variable *var = map_get(variables, lnode->name);
		Type *type = var->type;
		if (type->array_size != 0) {
			if (type->ty == INT){
				ptrnode = new_node('*', ptrnode, new_node_num(HALF_WORD));
			} else if (type->ty == LONG) {
				ptrnode = new_node('*', ptrnode, new_node_num(WORD));
			} else if (type->ty == PTR) {
				ptrnode = new_node('*', ptrnode, new_node_num(WORD));
			} else {
				error("unknown type!");
			}
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
		if (node->ty != ND_IDENT && node->ty != ND_INDIR && node->ty != ND_ARRAY)
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
			return new_node_num(HALF_WORD);

		if (node->ty == ND_ADDR) {
			Node *lnode = node->lhs;
			Variable *var = map_get(variables, lnode->name);
			Type *type = var->type;
			if (type->array_size != 0) {
				switch (type->ty) {
					case INT:
						return new_node_num(type->array_size * HALF_WORD);
					case LONG:
						return new_node_num(type->array_size * WORD);
					case PTR:
						return new_node_num(type->array_size * WORD);
				}
			}
			return new_node_num(WORD);
		}


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
					return new_node_num(HALF_WORD);
				case LONG:
					return new_node_num(WORD);
				case PTR:
					return new_node_num(WORD);
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

		// if declaring an array, get the array size
		Vlist *curr = tokens->next;
		if (((Token *)(curr->data))->ty == '[') {
			curr = curr->next;
			type->array_size = ((Token *)(curr->data))->val;
			if (type->array_size < 1)
				error_at(((Token *)(curr->data))->input, "should be >= 1!");
			curr = curr->next;
			if (((Token *)(curr->data))->ty != ']')
				error_at(((Token *)(tokens->data))->input, "should be ']'!");
			tokens->next = curr->next;
		}

		// compute variable or array size
		switch (type->ty) {
			case INT:
				*vcount += space(HALF_WORD);
				break;
			case LONG:
				*vcount += space(WORD);
				break;
			case PTR:
				*vcount += space(WORD);
		}

		Variable *var = new_var(*vcount, type);

		if (type->array_size) {
			switch (type->ty) {
				case INT:
					*vcount += (type->array_size - 1) * space(HALF_WORD);
					break;
				case LONG:
					*vcount += (type->array_size - 1) * space(WORD);
					break;
				case PTR:
					*vcount += (type->array_size - 1) * space(WORD);
		}
		}
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
			Type *type = var->type;
			if (consume('[')) {
				if (!type->array_size) {
					if (type->ty != PTR)
						error_at(((Token *)(tokens->data))->input, "deference a not pointer variable!");
					node = new_node('+', node, ptr(node));
					node = new_node(ND_INDIR, node, NULL);
				} else {
					node = new_node(ND_ARRAY, node, ptr(node));
				}
				if (!consume(']'))
					error_at(((Token *)(tokens->data))->input, "should be ']'!");
			} else if (type->array_size != 0) {
				node = new_node(ND_ADDR, node, NULL);
			}
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
