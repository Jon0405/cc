#include <stdio.h>

#include "cc.h"

extern Vlist *variables;
extern int vcount;
extern int lbegincount;
extern int lendcount;
extern int lelsecount;

void gen_lval(Node *node) {
	if (node->ty != ND_IDENT)
		error("should assign to a variable!");

	int var_place = *(int *)map_get(variables, node->name);
	int offset = (vcount - var_place) * 8;
	printf("  mov rax, rbp\n");
	printf("  sub rax, %d\n", offset);
	printf("  push rax\n");
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

	if (node->ty == ND_IF) {
		int end_num = lendcount++;
		gen(node->lhs);
		printf("  pop rax\n");
		printf("  cmp rax, 0\n");
		if (node->rhs->ty != ND_ELSE) {
			printf("  je .Lend%d\n", end_num);
			gen(node->rhs);
		} else {
			int else_num = lelsecount++;
			printf("  je .Lelse%d\n", else_num);
			gen(node->rhs->lhs);
			printf("  jmp .Lend%d\n", end_num);
			printf(".Lelse%d:\n", else_num);
			gen(node->rhs->rhs);
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

	if (node->ty == ND_NUM) {
		printf("  push %d\n", node->val);
		return;
	}

	if (node->ty == ND_IDENT) {
		gen_lval(node);
		printf("  pop rax\n");
		printf("  mov rax, [rax]\n");
		printf("  push rax\n");
		return;
	}

	if (node->ty == '=') {
		gen_lval(node->lhs);
		gen(node->rhs);

		printf("  pop rdi\n");
		printf("  pop rax\n");
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
