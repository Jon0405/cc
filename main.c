#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cc.h"

char *user_input;
Vlist *tokens;

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "argument number should be JUST 2!\n");
		return 1;
	}

	if (!strcmp(argv[1], "-test"))
		return runtest();

	// tokenize input
	user_input = argv[1];
	tokens = malloc(sizeof(Vlist));
	tokenize();
	// skip vlist head
	if (tokens->next == NULL) // empty vlist
		exit(1);
	tokens = tokens->next;
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
