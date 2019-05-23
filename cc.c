#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// for variable token
typedef struct {
        void **data;
        int capacity;
        int len;
} Vector;

Vector *new_vector() {
        Vector *vec = malloc(sizeof(Vector));
        vec->data = malloc(sizeof(void *) * 16);
        vec->capacity = 16;
        vec->len = 0;
        return vec;
}

void vec_push(Vector *vec, void *elem) {
        if (vec->capacity == vec->len) {
                vec->capacity *= 2;
                vec->data = realloc(vec->data, sizeof(void *) * vec->capacity);
        }
        vec->data[vec->len++] = elem;
}

// token type value table
enum {
        TK_NUM = 256, // integer
        TK_EQ,
        TK_NEQ,
        TK_BT,
        TK_BE,
        TK_LT,
        TK_LE,
        TK_EOF,       // end of input
};

// token structure
typedef struct {
        int ty;      // token type
        int val;     // if ty == TK_NUM, the value
        char *input; // for error messages
} Token;

// variable lengths tokens
Vector *tokens;

char *user_input; // raw input data

// print error messages like printf
void error(char *fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        vfprintf(stderr, fmt, ap);
        fprintf(stderr, "\n");
        exit(1);
}

// find error point
void error_at(char *loc, char *msg) {
        int err_pos = loc - user_input;
        fprintf(stderr, "%s\n", user_input);
        fprintf(stderr, "%*s", err_pos, ""); // output space
        fprintf(stderr, "^ %s\n", msg);
        exit(1);
}

// tokenize user_input
void tokenize() {
        char *p = user_input;

        while (*p) {
                // skip space
                if (isspace(*p)) {
                        p++;
                        continue;
                }

                if (*p == '!' || *p == '=' || *p == '>' || *p == '<') {
                        char pp = *p;
                        Token *token = malloc(sizeof(Token));
                        token->input = p;
                        p++;
                        if (*p == '=') {
                                switch (pp) {
                                        case '!':
                                                token->ty = TK_NEQ;
                                                break;
                                        case '=':
                                                token->ty = TK_EQ;
                                                break;
                                        case '>':
                                                token->ty = TK_BE;
                                                break;
                                        case '<':
                                                token->ty = TK_LE;
                                }
                                vec_push(tokens, token);
                                p++;
                                continue;
                        }
                        switch (pp) {
                                case '>':
                                        token->ty = TK_BT;
                                        break;
                                case '<':
                                        token->ty = TK_LT;
                        }
                        vec_push(tokens, token);
                        continue;
                }

                if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')') {
                        Token *token = malloc(sizeof(Token));
                        token->ty = *p;
                        token->input = p;
                        vec_push(tokens, token);
                        p++;
                        continue;
                }

                if (isdigit(*p)) {
                        Token *token = malloc(sizeof(Token));
                        token->ty = TK_NUM;
                        token->input = p;
                        token->val = strtol(p, &p, 10);
                        vec_push(tokens, token);
                        continue;
                }

                error_at(p, "tokenization failed!");
        }
        Token *token = malloc(sizeof(Token));
        token->ty = TK_EOF;
        token->input = p;
        vec_push(tokens, token);
}

enum {
        ND_NUM = 256, // integer node type
        ND_EQ,
        ND_NEQ,
        ND_LT,
        ND_LE,
};

typedef struct Node {
        int ty;           // operator or ND_NUM
        struct Node *lhs; // left side
        struct Node *rhs; // right side
        int val;          // only use this when ty == ND_NUM
} Node;

int pos; // position of token

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

int consume(int ty) {
        if (((Token *)(tokens->data[pos]))->ty != ty)
                return 0;
        pos++;
        return 1;
}

Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *term();

Node *expr() {
        return equality();
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
                        error_at(((Token *)(tokens->data[pos]))->input, "should be ')'!");
                return node;
        }

        // otherwise, it should be a value
        if (((Token *)(tokens->data[pos]))->ty == TK_NUM)
                return new_node_num(((Token *)(tokens->data[pos++]))->val);

        error_at(((Token *)(tokens->data[pos]))->input, "unexpected token");
}

void gen(Node *node) {
        if (node->ty == ND_NUM) {
                printf("  push %d\n", node->val);
                return;
        }

        gen(node->lhs);
        gen(node->rhs);

        printf("  pop rdi\n");
        printf("  pop rax\n");

        switch (node->ty) {
                case ND_EQ:
                        printf("  cmp rax, rdi\n");
                        printf("  sete al\n");
                        printf("  movzb rax, al\n");
                        break;
                case ND_NEQ:
                        printf("  cmp rax, rdi\n");
                        printf("  setne al\n");
                        printf("  movzb rax, al\n");
                        break;
                case ND_LT:
                        printf("  cmp rax, rdi\n");
                        printf("  setl al\n");
                        printf("  movzb rax, al\n");
                        break;
                case ND_LE:
                        printf("  cmp rax, rdi\n");
                        printf("  setle al\n");
                        printf("  movzb rax, al\n");
                        break;
                case '+':
                        printf("  add rax, rdi\n");
                        break;
                case '-':
                        printf("  sub rax, rdi\n");
                        break;
                case '*':
                        printf("  imul rdi\n");
                        break;
                case '/':
                        printf("  cqo\n");
                        printf("  idiv rdi\n");
        }

        printf("  push rax\n");
}

int expect(int line, int expected, int actual) {
        if (expected == actual)
                return 0;
        fprintf(stderr, "%d: %d expected, but got %d\n", line, expected, actual);
        exit(1);
}

int runtest() {
        Vector *vec = new_vector();
        expect(__LINE__, 0, vec->len);

        for (int i = 0; i < 100; i++) {
                int *data = malloc(sizeof(int));
                *data = i;
                vec_push(vec, data);
        }

        expect(__LINE__, 100, vec->len);
        expect(__LINE__, 0, *((int *)vec->data[0]));
        expect(__LINE__, 50, *((int *)vec->data[50]));
        expect(__LINE__, 99, *((int *)vec->data[99]));

        printf("OK\n");
        return 0;
}

int main(int argc, char **argv) {
        if (argc != 2) {
                fprintf(stderr, "argument number should be JUST 2!\n");
                return 1;
        }

        if (!strcmp(argv[1], "-test"))
                return runtest();

        // tokenize input
        user_input = argv[1];
        tokens = new_vector();
        tokenize();
        Node *node = expr();

        // header
        printf(".intel_syntax noprefix\n");
        printf(".global main\n");

        printf("main:\n");

        // recursively constract the abstract syntax tree and generate assembly code
        gen(node);

        // pop the result
        printf("  pop rax\n");

        // return to shell
        printf("  ret\n");

        return 0;
}
