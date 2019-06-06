#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cc.h"

char *user_input;
Vlist *tokens;
Vlist *code;
Vlist *variables;
Vlist *types;
Vlist *functions;
int vcount;      // variable count
int lbegincount; // begin label count
int lendcount;   // end label count
int lelsecount;  // else label count

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
	lbegincount = 0;

	// tokenize input
	user_input = argv[1];
	tokens = new_vlist();
	tokenize();
	if (tokens->next == NULL) // empty vlist
		exit(1);
	tokens = tokens->next; // skip head
	functions = new_vlist();
	code = variables = NULL;
	program();
	if (code->next == NULL) // empty vlist
		exit(1);

	// header
	printf(".intel_syntax noprefix\n");

	functions = functions->next; // skip list head
	for (;;) {
		if (functions == NULL)
			break;

		Func *func = (Func *)functions->data;
		code = func->code;
		code = code->next; // skip list head
		variables = func->variables;
		types = func->types;
		
		printf(".global %s\n", func->name);
		printf("%s:\n", func->name);

		// get variables space
		printf("  push rbp\n");
		printf("  mov rbp, rsp\n");
		printf("  sub rsp, %d\n", vcount * 8);

		// set arguments to variables
		gen(func->nodedef);

		// generate assembly code
		for (;;) {
			if (code == NULL)
				break;

			gen((Node *)(code->data));
			code = code->next;
		}

		code = variables = types = NULL;
		functions = functions->next;
	}


	return 0;
}
