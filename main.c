#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cc.h"

char *user_input; // raw input data

// for tokenizer
Vlist *tokens; // token list

// for parser
int *vcount;        // variable space count
Vlist *variables;   // variable map
Vlist *globals;     // global variables map
Vlist *functions;   // function list
Vlist *return_type; // functions return typ mape

// for assembly code generator
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
	return_type = new_vlist();
	globals = new_vlist();
	variables = NULL;
	program();

	// header
	printf(".intel_syntax noprefix\n");

	// allocate global variables
	Vlist *curr_var = globals->next; // skip list head
	while (curr_var != NULL) {
		char *name = ((Map *)curr_var->data)->key;
		Variable *var = (Variable *)((Map *)curr_var->data)->val;
		printf(".comm %s,%d,%d\n", name, var->place * 4, var->place * 4);
		curr_var = curr_var->next;
	}

	functions = functions->next; // skip list head
	for (;;) {
		if (functions == NULL)
			break;

		// initialization
		Func *func = (Func *)functions->data;
		vcount = func->vcount;
		variables = func->variables;
		
		printf(".global %s\n", func->name);
		printf("%s:\n", func->name);

		// get variables space
		printf("  push rbp\n");
		printf("  mov rbp, rsp\n");
		printf("  sub rsp, %d\n", *vcount * 4);

		// set arguments to variables
		gen(func->nodedef);

		// generate assembly code
		gen(func->code);

		variables = NULL; vcount = NULL; // leave from the function
		functions = functions->next;
	}


	return 0;
}
