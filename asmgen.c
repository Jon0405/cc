#include <stdio.h>

#include "cc.h"

char *reg_names[6] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

extern Vlist *variables;
extern int vcount;
extern int lbegincount;
extern int lendcount;
extern int lelsecount;

void gen_lval(Node *node) {
	if (node->ty != ND_IDENT)
		error("should assign to a variable!");

	int var_place = *(int *)map_get(variables, node->name);
	int offset = (vcount - var_place + 1) * 8;
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
		Vlist *curr = node->stmts;
		if (curr->next == NULL)
			error("empty statement!");
		curr = curr->next; // skip list head
		for (;;) {
			gen((Node *)curr->data);
			curr = curr->next;
			if (curr == NULL)
				break;
		}
		return;
	}

	if (node->ty == ND_CALL) {
		int regcount = 0;
		Vlist *curr = node->argv->next; // skip list head
		while (curr != NULL && regcount < 6) {
			Node *nodearg = (Node *)curr->data;
			gen(nodearg);
			printf("  pop %s\n", reg_names[regcount++]);
			curr = curr->next;
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
		Vlist *curr = node->argv->next; // skip list head
		while (curr != NULL && regcount < 6) {
			Node *nodearg = (Node *)curr->data;
			gen_lval(nodearg);
			printf("  push %s\n", reg_names[regcount++]);
			printf("  pop rdi\n");
			printf("  pop rax\n");
			printf("  mov [rax], rdi\n");
			curr = curr->next;
		}
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
