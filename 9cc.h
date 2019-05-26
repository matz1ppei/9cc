#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
  TK_NUM = 256, // 整数トークン
  TK_EQ,        // EQual
  TK_NE,        // Not Equal
  TK_LE,        // Less than or Equal
  TK_GE,        // Greater than or Equal
  TK_EOF,       // 入力の終わりを表すトークン
};

typedef struct {
  int ty;      // トークンの型
  int val;     // tyがTK_NUMの場合、その数値
  char *input; // トークン文字列（エラーメッセージ用）
} Token;

enum {
  ND_NUM = 256, // トークンの型
};

typedef struct Node {
  int ty;           // 演算子かND_NUM
  struct Node *lhs; // 左辺
  struct Node *rhs; // 右辺
  int val;          // tyがND_NUMの場合のみ使う
} Node;

typedef struct {
  void **data;
  int capacity;
  int len;
} Vector;

Vector *tokens;

// 9cc.c
void error(char *fmt, ...);

// codegen.c
void gen(Node *node);

// container.c
void vec_push(Vector *vec, void *elem);
Vector *new_vector();
void runtest();

// parse.c
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *term();

// token.c
void tokenize(char *p);
