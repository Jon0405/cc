#ifndef _CC_H
#define _CC_H

// for variable token
typedef struct Vlist{
	void *data;
	struct Vlist *next;
} Vlist;

// token type value table
enum {
	TK_NUM = 256, // integer
	TK_EQ,
	TK_NEQ,
	TK_BT,
	TK_BE,
	TK_LT,
	TK_LE,
	TK_IDENT,     // identifier
	TK_EOF,       // end of input
};

// token structure
typedef struct {
	int ty;      // token type
	int val;     // if ty == TK_NUM, the value
	char *input; // for error messages
} Token;

// node type value table
enum {
	ND_NUM = 256, // integer node type
	ND_EQ,
	ND_NEQ,
	ND_LT,
	ND_LE,
	ND_IDENT,     // indentifier
};

// node structure
typedef struct Node {
	int ty;		  // operator or ND_NUM
	struct Node *lhs; // left side
	struct Node *rhs; // right side
	int val;	  // only use this when ty == ND_NUM
	char name;        // only use this when ty == ND_IDENT
} Node;

// error
void error(char *fmt, ...);
void error_at(char *loc, char *msg);

// vector operations
void vlist_push(Vlist *vlist, void *data);

// tokenization
void tokenize();

// node operations
Node *new_node(int ty, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *new_node_ident(int val);

// parsing
void program();
Node *add();
Node *assign();
Node *equality();
Node *expr();
Node *mul();
Node *relational();
Node *stmt();
Node *term();
Node *unary();

// generate assembly code
void gen(Node *node);

// test functions
int runtest();

#endif
