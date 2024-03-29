#ifndef _CC_H
#define _CC_H

// space (in bytes)
#define BYTE       1
#define WORD       BYTE * 8
#define HALF_WORD  WORD / 2
#define SPACE_SIZE BYTE

#define space(len) (len / SPACE_SIZE)

// linked list
typedef struct Vlist{
	void *data;
	struct Vlist *next;
} Vlist;

// map structure
typedef struct {
	char *key;
	void *val;
} Map;

// type
enum {
	CHAR,
	INT,
	LONG,
	PTR,
};

typedef struct Type {
	int ty;
	struct Type *ptrof;
	size_t array_size;
} Type;

typedef struct {
	int place;
	Type *type;
} Variable;

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
	TK_SIZEOF,
	TK_CHAR,
	TK_INT,
	TK_LONG,
};

// token structure
typedef struct {
	int ty;      // token type
	int val;     // if ty == TK_NUM, the value
	int varty;   // if ty == TK_TYPE, the variable typee
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
	ND_ADDR,      // address
	ND_INDIR,     // indirect
	ND_IDENT,     // indentifier
	ND_GLOBAL,    // global variable
	ND_RET,       // return
	ND_IF,        // if
	ND_ELSE,      // else
	ND_WHILE,     // while
	ND_FOR,       // for loop
	ND_FOR_INIT,
	ND_FOR_COND,
	ND_BLOCK,     // code block
	ND_CALL,      // function call
	ND_DEF,       // function definition
	ND_NOP,
	ND_ARRAY,
};

// node structure
typedef struct Node {
	int ty;		  // operator or ND_NUM
	struct Node *lhs; // left side
	struct Node *rhs; // right side
	int val;	  // only use this when ty == ND_NUM
	char *name;       // only use this when ty == ND_IDENT or ty == ND_CALL
	Vlist *stmts;     // only use this when ty == ND_BLOCK
	Vlist *argv;      // only use this when ty == ND_CALL or ty == ND_DEF
} Node;

// function structure
typedef struct {
	char *name;
	int *vcount;
	Vlist *variables;
	Node *code;
	Node *nodedef;    // for arguments
} Func;

// exported functions

// error
void error(char *fmt, ...);
void error_at(char *loc, char *msg);

// vlist
Vlist *new_vlist();
void vlist_push(Vlist *vlist, void *data);

// map
void map_put(Vlist *map, char *key, void *val);
void *map_get(Vlist *map, char *key);

// token
void tokenize();

// variable
Variable *new_var(int place, Type *type);

// type
Type *new_type(int ty, Type *ptfof);

// node
Node *new_node(int ty, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *new_node_ident(char *name);
Node *new_node_global(char *name);
Node *new_node_call(char *name);
Node *new_node_funcdef(char *name);
Node *new_node_block(Vlist *stmts);

// parse
void program();

// generate assembly code
void gen(Node *node);

// test functions
int runtest();

#endif
