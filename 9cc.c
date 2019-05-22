#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// トークンの型を表す値
enum {
  TK_NUM = 256, // 整数トークン
  TK_EQ,        // EQual
  TK_NE,        // Not Equal
  TK_LE,        // Less than or Equal
  TK_GE,        // Greater than or Equal
  TK_EOF,       // 入力の終わりを表すトークン
};

// トークンの型
typedef struct {
  int ty;      // トークンの型
  int val;     // tyがTK_NUMの場合、その数値
  char *input; // トークン文字列（エラーメッセージ用）
} Token;

// ノードの型を表す値
enum {
  ND_NUM = 256, // トークンの型
};

// ノードの型
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

// トークナイズした結果のトークン列はVectorに保存する
Vector *tokens;

// トークンの読み取り位置
int pos = 0;

// エラーを報告するための関数
// printfと同じ引数を取る
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// 関数プロトタイプ
Node *term();
Node *mul();
Node *add();
Node *unary();
Node *expr();
Node *equality();
Node *relational();
void add_token();
void add_token_num();
void vec_push();

// pが指している文字列をトークンに分割してtokensに保存する
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

Node *new_node(int ty, Node *lhs, Node *rhs) {
  Node *node = malloc(sizeof(Node));
  node->ty  = ty;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = malloc(sizeof(Node));
  node->ty  = ND_NUM;
  node->val = val;
  return node;
}

Vector *new_vector() {
  Vector *vec = malloc(sizeof(Vector));
  vec->data = malloc(sizeof(void *) * 16);
  vec->capacity = 16;
  vec->len = 0;
  return vec;
}

void vec_push(Vector *vec, void *elem) {
  if (vec->capacity == vec->len) {
    vec->capacity *= 2;
    vec->data = realloc(vec->data, sizeof(void *) * vec->capacity);
  }

  vec->data[vec->len++] = elem;
}

int consume(int ty) {
  Token *token = tokens->data[pos];
  if (token->ty != ty)
    return 0;
  pos++;
  return 1;
}

Node *expr() {
  Node *node = equality();
  return node;
}

Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume(TK_EQ))
      node = new_node(TK_EQ, node, relational());
    else if (consume(TK_NE))
      node = new_node(TK_NE, node, relational());
    else
      return node;
  }
}

Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume('<'))
      node = new_node('<', node, add());
    else if (consume(TK_LE))
      node = new_node(TK_LE, node, add());
    else if (consume('>'))
      node = new_node('<', add(), node);
    else if (consume(TK_GE))
      node = new_node(TK_LE, add(), node);
    else
      return node;
  }
}

Node *term() {
  Token *token = tokens->data[pos];
  // 次のトークンが'('なら、"(" add ")"のはず
  if (consume('(')) {
    Node *node = add();
    if (!consume(')'))
      error("開きカッコに対応する閉じカッコがありません： %s",
            token->input);
    return node;
  }

  // そうでなければ数値のはず
  if (token->ty == TK_NUM) {
    int val = token->val;
    pos++;
    return new_node_num(val);
  }

  error("数値でも開きカッコでもないトークンです： %s",
         token->input);
}

Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume('*'))
      node = new_node('*', node, unary());
    else if (consume('/'))
      node = new_node('/', node, unary());
    else
      return node;
  }
}

Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume('+'))
      node = new_node('+', node, mul());
    else if (consume('-'))
      node = new_node('-', node, mul());
    else
      return node;
  }
}

Node *unary() {
  if (consume('+'))
    return term();
  if (consume('-'))
    return new_node('-', new_node_num(0), term());
  return term();
}

void gen(Node *node) {
  if (node->ty == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->ty) {
  case TK_EQ:
    printf("  cmp rax, rdi\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    break;
  case TK_NE:
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;
  case '<':
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    break;
  case TK_LE:
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
    printf("  mul rdi\n");
    break;
  case '/':
    printf("  mov rdx, 0\n");
    printf("  div rdi\n");
  }

  printf("  push rax\n");
}

void expect(int line, int expected, int actual) {
  if (expected == actual)
    return;
  fprintf(stderr, "%d: %d expected, but got %d\n",
          line, expected, actual);
  exit(1);
}

void runtest() {
  Vector *vec = new_vector();
  expect(__LINE__, 0, vec->len);

  for (int i = 0; i < 100; i++)
    vec_push(vec, (void *)i);

  expect(__LINE__, 100, vec->len);
  expect(__LINE__,   0, (long)vec->data[0]);
  expect(__LINE__,  50, (long)vec->data[50]);
  expect(__LINE__,  99, (long)vec->data[99]);

  printf("OK\n");
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  if (!strncmp(argv[1], "-test", 5)) {
    runtest();
    return 0;
  }

  // トークナイズしてパースする
  tokens = new_vector();
  tokenize(argv[1]);
  Node *node = expr();

  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // 抽象構文木を下りながらコード生成
  gen(node);

  // スタックトップに式全体の値が残っているはずなので
  // それをRAXにロードして関数からの返り値とする
  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}