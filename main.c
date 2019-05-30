#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cc.h"

char *user_input;
Vlist *tokens;
Vlist *code;
Vlist *variables;
int vcount;     // variable count
int lendcount;  // end label count
int lelsecount; // else label count

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "argument number should be JUST 2!\n");
		return 1;
	}

	if (!strcmp(argv[1], "-test"))
		return runtest();

	// initializaion
	vcount = 0;
	lendcount = 0;
	lelsecount = 0;

	// tokenize input
	user_input = argv[1];
	tokens = new_vlist();
	tokenize();
	if (tokens->next == NULL) // empty vlist
		exit(1);
	tokens = tokens->next; // skip head
	code = new_vlist();
	variables = new_vlist();
	program();
	if (code->next == NULL) // empty vlist
		exit(1);
	code = code->next; // skip head

	// header
	printf(".intel_syntax noprefix\n");
	printf(".global main\n");

	printf("main:\n");

	// get variables space
	printf("  push rbp\n");
	printf("  mov rbp, rsp\n");
	printf("  sub rsp, %d\n", vcount * 8);

	// generate assembly code
	for (;;) {
		if (code == NULL)
			break;

		gen((Node *)(code->data));
		code = code->next;

		// pop the result
		printf("  pop rax\n");
	}

	// return to shell
	printf("  mov rsp, rbp\n");
	printf("  pop rbp\n");
	printf("  ret\n");

	return 0;
}
