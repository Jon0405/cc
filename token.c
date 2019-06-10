#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "cc.h"

extern char *user_input;
extern Vlist *tokens;

int is_alnum(char c) {
	return ('a' <= c && c <='z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || (c == '_');
}

// tokenize user_input
void tokenize() {
	char *p = user_input;

	while (*p) {
		// skip space
		if (isspace(*p)) {
			p++;
			continue;
		}

		if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
			Token *token = malloc(sizeof(Token));
			token->ty = TK_RET;
			token->input = p;
			vlist_push(tokens, token);
			p += 6;
			continue;
		}

		if (strncmp(p, "if", 2) == 0 && !is_alnum(p[2])) {
			Token *token = malloc(sizeof(Token));
			token->ty = TK_IF;
			token->input = p;
			vlist_push(tokens, token);
			p += 2;
			continue;
		}

		if (strncmp(p, "else", 4) == 0 && !is_alnum(p[4])) {
			Token *token = malloc(sizeof(Token));
			token->ty = TK_ELSE;
			token->input = p;
			vlist_push(tokens, token);
			p += 4;
			continue;
		}

		if (strncmp(p, "while", 5) == 0 && !is_alnum(p[5])) {
			Token *token = malloc(sizeof(Token));
			token->ty = TK_WHILE;
			token->input = p;
			vlist_push(tokens, token);
			p += 5;
			continue;
		}

		if (strncmp(p, "for", 3) == 0 && !is_alnum(p[3])) {
			Token *token = malloc(sizeof(Token));
			token->ty = TK_FOR;
			token->input = p;
			vlist_push(tokens, token);
			p += 3;
			continue;
		}

		if (strncmp(p, "int", 3) == 0 && !is_alnum(p[3])) {
			Token *token = malloc(sizeof(Token));
			token->ty = TK_INT;
			token->input = p;
			vlist_push(tokens, token);
			p += 3;
			continue;
		}

		if (strchr("=!><", *p)) {
			char pp = *p;
			Token *token = malloc(sizeof(Token));
			token->input = p;
			p++;
			if (*p == '=') {
				switch (pp) {
					case '!':
						token->ty = TK_NEQ;
						break;
					case '=':
						token->ty = TK_EQ;
						break;
					case '>':
						token->ty = TK_BE;
						break;
					case '<':
						token->ty = TK_LE;
				}
				p++;
				vlist_push(tokens, token);
				continue;
			}
			switch (pp) {
				case '>':
					token->ty = TK_BT;
					break;
				case '<':
					token->ty = TK_LT;
					break;
				case '=':
					token->ty = '=';
			}
			vlist_push(tokens, token);
			continue;
		}

		if (strchr("+-*/();{},&", *p)) {
			Token *token = malloc(sizeof(Token));
			token->ty = *p;
			token->input = p;
			vlist_push(tokens, token);
			p++;
			continue;
		}

		if (isdigit(*p)) {
			Token *token = malloc(sizeof(Token));
			token->ty = TK_NUM;
			token->input = p;
			token->val = strtol(p, &p, 10);
			vlist_push(tokens, token);
			continue;
		}

		if ('a' <= *p && *p <= 'z') {
			char *start = p;
			int len = 1;
			while (isalnum(*(++p)))
				len++;
			char *name = malloc(len * sizeof(char));
			memcpy(name, start, len * sizeof(char));

			Token *token = malloc(sizeof(Token));
			token->ty = TK_IDENT;
			token->input = p;
			token->name = name;
			vlist_push(tokens, token);
			continue;
		}

		error_at(p, "tokenization failed!");
	}
	Token *token = malloc(sizeof(Token));
	token->ty = TK_EOF;
	token->input = p;
	vlist_push(tokens, token);
}
