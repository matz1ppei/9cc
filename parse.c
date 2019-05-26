#include "9cc.h"

int pos = 0;

int consume(int ty) {
  Token *token = tokens->data[pos];
  if (token->ty != ty)
    return 0;
  pos++;
  return 1;
}

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

Node *unary() {
  if (consume('+'))
    return term();
  if (consume('-'))
    return new_node('-', new_node_num(0), term());
  return term();
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
