#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// token type value table
enum {
        TK_NUM = 256, // integer
        TK_EOF,       // > 256 in ASCII, end of input
};

// token structure
typedef struct {
        int ty;      // token type
        int val;     // if ty == TK_NUM, the value
        char *input; // for error messages
} Token;

// assume input is less than 100 tokens
Token tokens[100];

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

        int i = 0;
        while (*p) {
                // skip space
                if (isspace(*p)) {
                        p++;
                        continue;
                }

                if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')') {
                        tokens[i].ty = *p;
                        tokens[i].input = p;
                        i++;
                        p++;
                        continue;
                }

                if (isdigit(*p)) {
                        tokens[i].ty = TK_NUM;
                        tokens[i].input = p;
                        tokens[i].val = strtol(p, &p, 10);
                        i++;
                        continue;
                }

                error_at(p, "tokenization failed!");
        }
        tokens[i].ty = TK_EOF;
        tokens[i].input = p;
}

enum {
        ND_NUM = 256, // integer node type
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
        if (tokens[pos].ty != ty)
                return 0;
        pos++;
        return 1;
}

Node *expr();
Node *mul();
Node *unary();
Node *term();

Node *expr() {
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
                        error_at(tokens[pos].input, "should be ')'!");
                return node;
        }

        // otherwise, it should be a value
        if (tokens[pos].ty == TK_NUM)
                return new_node_num(tokens[pos++].val);

        error_at(tokens[pos].input, "unexpected token");
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

int main(int argc, char **argv) {
        if (argc != 2) {
                fprintf(stderr, "argument number should be JUST 2!\n");
                return 1;
        }

        // tokenize input
        user_input = argv[1];
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
