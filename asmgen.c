#include <stdio.h>

#include "cc.h"

#define ARGC_MAX 6

char *reg_names[ARGC_MAX] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

extern Vlist *variables;
extern int *vcount;
extern int lbegincount;
extern int lendcount;
extern int lelsecount;

Type *gen_lval(Node *node) {
	if (node->ty == ND_INDIR) {
		Type *type = gen_lval(node->lhs);
		type = type->ptrof;
		printf("  pop rax\n");
		printf("  mov rax, [rax]\n");
		printf("  push rax\n");
		return type;
	}
	if (node->ty == ND_IDENT) {
		Variable *var = (Variable *)map_get(variables, node->name);
		int size = 0;
		switch (var->type->ty) {
			case INT:
				size = 1;
				break;
			case LONG:
				size = 2;
				break;
			case PTR:
				size = 2;
		}
		int offset = (*vcount - var->place + size) * 4;
		printf("  mov rax, rbp\n");
		printf("  sub rax, %d\n", offset);
		printf("  push rax\n");
		return var->type;
	}
	
	error("should be a variable or a variable pointer!");
	return NULL;
}

void gen(Node *node) {
	if (node->ty == ND_RET) {
		gen(node->lhs);
		printf("  pop rax\n");
		printf("  mov rsp, rbp\n");
		printf("  pop rbp\n");
		printf("  ret\n");
		return;
	}

	if (node->ty == ND_NOP)
		return;

	if (node->ty == ND_IF) {
		int end_num = lendcount++;
		gen(node->lhs);
		printf("  pop rax\n");
		printf("  cmp rax, 0\n");
		if (node->rhs->ty != ND_ELSE) {
			printf("  je .Lend%d\n", end_num);
			gen(node->rhs);
		} else {
			Node *nodeelse = node->rhs;
			int else_num = lelsecount++;
			printf("  je .Lelse%d\n", else_num);
			gen(nodeelse->lhs);
			printf("  jmp .Lend%d\n", end_num);
			printf(".Lelse%d:\n", else_num);
			gen(nodeelse->rhs);
		}
		printf(".Lend%d:\n", end_num);
		return;
	}

	if (node->ty == ND_WHILE) {
		int begin_num = lbegincount++;
		int end_num = lendcount++;
		printf(".Lbegin%d:\n", begin_num);
		gen(node->lhs);
		printf("  pop rax\n");
		printf("  cmp rax, 0\n");
		printf("  je .Lend%d\n", end_num);
		gen(node->rhs);
		printf("  jmp .Lbegin%d\n", begin_num);
		printf(".Lend%d:\n", end_num);
		return;
	}

	if (node->ty == ND_FOR) {
		int begin_num = lbegincount++;
		int end_num = lendcount++;
		Node *nodeinit = node->lhs;
		if (nodeinit->ty != ND_FOR_INIT)
			error("not a vaild for loop format!");
		Node *nodecond = nodeinit->rhs;
		if (nodecond->ty != ND_FOR_COND)
			error("not a vaild for loop format!");
		gen(nodeinit->lhs); // init
		printf(".Lbegin%d:\n", begin_num);
		gen(nodecond->lhs); // cond
		printf("  pop rax\n");
		printf("  cmp rax, 0\n");
		printf("  je .Lend%d\n", end_num);
		gen(node->rhs); // loop body
		gen(nodecond->rhs); // increment
		printf("  jmp .Lbegin%d\n", begin_num);
		printf(".Lend%d:", end_num);
		return;
	}

	if (node->ty == ND_BLOCK) {
		if (node->stmts->next == NULL)
			error("empty block!");
		for (Vlist *curr = node->stmts->next; curr != NULL; curr = curr->next)
			gen((Node *)curr->data);
		return;
	}

	if (node->ty == ND_CALL) {
		int regcount = 0;
		for (Vlist *curr = node->argv->next; curr != NULL && regcount < ARGC_MAX; curr = curr->next) {
			Node *nodearg = (Node *)curr->data;
			gen(nodearg);
			printf("  pop %s\n", reg_names[regcount++]);
		}
		printf("  push rbp\n");
		printf("  mov rbp, rsp\n");
		printf("  and rsp, -0x10\n");
		printf("  call %s\n", node->name);
		printf("  mov rsp, rbp\n");
		printf("  pop rbp\n");
		printf("  push rax\n");
		return;
	}

	if (node->ty == ND_DEF) {
		int regcount = 0;
		for (Vlist *curr = node->argv->next; curr != NULL && regcount < ARGC_MAX; curr = curr->next) {
			Node *nodearg = (Node *)curr->data;
			Type *type = gen_lval(nodearg);
			printf("  push %s\n", reg_names[regcount++]);
			printf("  pop rdi\n");
			printf("  pop rax\n");
			if (type->ty == INT)
				printf("  mov DWORD PTR [rax], edi\n");
			else	
				printf("  mov [rax], rdi\n");
		}
		return;
	}

	if (node->ty == ND_NUM) {
		printf("  push %d\n", node->val);
		return;
	}

	if (node->ty == ND_ADDR) {
		gen_lval(node->lhs);
		return;
	}

	if (node->ty == ND_INDIR) {
		gen(node->lhs);
		printf("  pop rax\n");
		printf("  mov rax, [rax]\n");
		printf("  push rax\n");
		return;
	}

	if (node->ty == ND_IDENT) {
		Type *type = gen_lval(node);
		printf("  pop rax\n");
		if (type->ty == INT)
			printf("  mov eax, DWORD PTR [rax]\n");
		else
			printf("  mov rax, [rax]\n");
		printf("  push rax\n");
		return;
	}

	if (node->ty == '=') {
		Type *type = gen_lval(node->lhs);
		gen(node->rhs);

		printf("  pop rdi\n");
		printf("  pop rax\n");
		if (type->ty == INT)
			printf("  mov DWORD PTR [rax], edi\n");
		else	
			printf("  mov [rax], rdi\n");
		printf("  push rdi\n");
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
