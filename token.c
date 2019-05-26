#include "9cc.h"

void add_token(int ty, char *input) {
  Token *token = malloc(sizeof(Token));
  token->ty = ty;
  token->input = input;
  vec_push(tokens, token);
};

void add_token_num(int ty, char *input, int val) {
  Token *token = malloc(sizeof(Token));
  token->ty = ty;
  token->val = val;
  token->input = input;
  vec_push(tokens, token);
};

void tokenize(char *p) {
  int i = 0;
  while (*p) {
    // 空白文字をスキップ
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (!strncmp(p, "==", 2)) {
      add_token(TK_EQ, p);
      p += 2;
      i++;
      continue;
    } else if (!strncmp(p, "!=", 2)) {
      add_token(TK_NE, p);
      p += 2;
      i++;
      continue;
    } else if (!strncmp(p, "<=", 2)) {
      add_token(TK_LE, p);
      p += 2;
      i++;
      continue;
    } else if (!strncmp(p, ">=", 2)) {
      add_token(TK_GE, p);
      p += 2;
      i++;
      continue;
    }

    if (strchr("+-*/()<>", *p)) {
      add_token(*p, p);
      i++;
      p++;
      continue;
    }

    if (isdigit(*p)) {
      add_token_num(TK_NUM, p, strtol(p, &p, 10));
      i++;
      continue;
    }

    error("トークナイズできません： %s", p);
    exit(1);
  }

  add_token(TK_EOF, p);
}
