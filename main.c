#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cc.h"

char *user_input;
Vlist *tokens;
Node *code[100];

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
	program();

	// header
	printf(".intel_syntax noprefix\n");
	printf(".global main\n");

	printf("main:\n");

	// get 26 variables space
	printf("  push rbp\n");
	printf("  mov rbp, rsp\n");
	printf("  sub rsp, 208\n");

	// generate assembly code
	for (int i = 0; code[i]; i++) {
		gen(code[i]);

		// pop the result
		printf("  pop rax\n");
	}

	// return to shell
	printf("  mov rsp, rbp\n");
	printf("  pop rbp\n");
	printf("  ret\n");

	return 0;
}
