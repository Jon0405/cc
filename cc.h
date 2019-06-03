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
	TK_RET,       // return
	TK_IF,        // if
	TK_ELSE,      // else
	TK_WHILE,     // while
	TK_FOR,       // for
};

// token structure
typedef struct {
	int ty;      // token type
	int val;     // if ty == TK_NUM, the value
	char *name;  // if ty == TK_IDENT, the identifier name
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
	ND_RET,       // return
	ND_IF,        // if
	ND_ELSE,      // else
	ND_WHILE,     // while
	ND_FOR,       // for loop
	ND_FOR_INIT,
	ND_FOR_COND,
};

// node structure
typedef struct Node {
	int ty;		  // operator or ND_NUM
	struct Node *lhs; // left side
	struct Node *rhs; // right side
	int val;	  // only use this when ty == ND_NUM
	char *name;       // only use this when ty == ND_IDENT
} Node;

// map structure
typedef struct {
	char *key;
	void *val;
} Map;

// export functions
// error
void error(char *fmt, ...);
void error_at(char *loc, char *msg);

// vector operations
Vlist *new_vlist();
void vlist_push(Vlist *vlist, void *data);

// tokenization
void tokenize();

// node operations
Node *new_node(int ty, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *new_node_ident(char *name);

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

// map
void map_put(Vlist *map, char *key, void *val);
void *map_get(Vlist *map, char *key);

#endif
