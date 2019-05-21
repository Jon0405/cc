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

char *user_input;

// assume input is less than 100 tokens
Token tokens[100];

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
	int pos = loc - user_input;
	fprintf(stderr, "%s\n", user_input);
	fprintf(stderr, "%*s", pos, ""); // output space
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

		if (*p == '+' || *p == '-') {
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

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "argument number should be JUST 2!\n");
		return 1;
	}

	// tokenize input
	user_input = argv[1];
	tokenize();

	printf(".intel_syntax noprefix\n");
	printf(".global main\n");

	printf("main:\n");

	// first token should be an integer
	if (tokens[0].ty != TK_NUM)
		error_at(tokens[0].input, "not an integer");
	printf("  mov rax, %d\n", tokens[0].val);

	// process by finding '+' or '-'
	int i = 1;
	while (tokens[i].ty != TK_EOF) {
		if (tokens[i].ty == '+') {
			i++;
			if (tokens[i].ty != TK_NUM)
				error_at(tokens[i].input, "not an integer");
			printf("  add rax, %d\n", tokens[i].val);
			i++;
			continue;
		}

		if (tokens[i].ty == '-') {
			i++;
			if (tokens[i].ty != TK_NUM)
				error_at(tokens[i].input, "not an integer");
			printf("  sub rax, %d\n", tokens[i].val);
			i++;
			continue;
		}

		error_at(tokens[i].input, "unexpected token");
	}

	printf("  ret\n");

	return 0;
}
