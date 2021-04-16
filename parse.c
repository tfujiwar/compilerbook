#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "9cc.h"

bool consume_token(int kind) {
  if (token->kind != kind)
    return false;

  token = token->next;
  return true;
}

bool consume(char *op) {
  if ((token->kind != TK_RESERVED) || strlen(op) != token->len || memcmp(token->str, op, token->len))
    return false;

  token = token->next;
  return true;
}

Token *consume_ident() {
  if (token->kind != TK_IDENT || !is_token_char(token->str[0]))
    return NULL;

  Token *tok = token;
  token = token->next;
  return tok;
}

LVar *find_lvar(Token *tok) {
  for (LVar *var = locals; var; var = var->next)
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
      return var;
  return NULL;
}

bool expect_token(int kind) {
  if (token->kind != kind)
    error_at(token->str, "expected %d, but got %d", kind, token->kind);
  token = token->next;
}

void expect(char *op) {
  if ((token->kind != TK_RESERVED) || strlen(op) != token->len || memcmp(token->str, op, token->len))
    error_at(token->str, "not a '%s'", op);
  token = token->next;
}

int expect_number() {
  if (token->kind != TK_NUM)
    error_at(token->str, "not a 'number'");
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() {
  return token->kind == TK_EOF;
}

bool is_token_char(char c) {
  return ('a' <= c && c <= 'z') ||
         ('A' <= c && c <= 'Z') ||
         ('0' <= c && c <= '9') ||
         (c == '_');
}

Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

Token *tokenize() {
  char *p = user_input;
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (memcmp(p, "int", 3) == 0) {
      cur = new_token(TK_INT, cur, p, 3);
      p += 3;
      continue;
    }

    if (memcmp(p, "==", 2) == 0 || memcmp(p, "!=", 2) == 0 || memcmp(p, "<=", 2) == 0 || memcmp(p, ">=", 2) == 0) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    if (strchr("+-*/()<>{}[]=;,*&", *p)) {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    if (isdigit(*p)) {
      char* prev = p;
      cur = new_token(TK_NUM, cur, p, 0);
      cur->val = strtol(p, &p, 10);
      cur->len = p - prev;
      continue;
    }

    if (memcmp(p, "return", 6) == 0 && !is_token_char(p[6])) {
      cur = new_token(TK_RETURN, cur, p, 6);
      p += 6;
      continue;
    }

    if (memcmp(p, "if", 2) == 0 && !is_token_char(p[2])) {
      cur = new_token(TK_IF, cur, p, 2);
      p += 2;
      continue;
    }

    if (memcmp(p, "else", 4) == 0 && !is_token_char(p[4])) {
      cur = new_token(TK_ELSE, cur, p, 4);
      p += 4;
      continue;
    }

    if (memcmp(p, "for", 3) == 0 && !is_token_char(p[3])) {
      cur = new_token(TK_FOR, cur, p, 3);
      p += 3;
      continue;
    }

    if (memcmp(p, "while", 5) == 0 && !is_token_char(p[5])) {
      cur = new_token(TK_WHILE, cur, p, 5);
      p += 5;
      continue;
    }

    if (memcmp(p, "sizeof", 6) == 0 && !is_token_char(p[6])) {
      cur = new_token(TK_SIZEOF, cur, p, 6);
      p += 6;
      continue;
    }

    if (is_token_char(*p)) {
      cur = new_token(TK_IDENT, cur, p, 0);
      while (is_token_char(*p)) {
        p++;
        cur->len++;
      }
      continue;
    }

    error_at(p, "failed to tokenize");
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_expr(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;

  if (lhs && rhs) {
    if (lhs) node->type = lhs->type;
    else if (rhs) node->type = lhs->type;
  }

  if (kind == ND_DEREF) {
    node->type = lhs->type->ptr_to;
  }

  if (kind == ND_ADDR) {
    node->type = calloc(1, sizeof(Type));
    node->type->ty = PTR,
    node->type->size = 8;
    node->type->ptr_to = lhs->type;
  }

  return node;
}

Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  node->type = calloc(1, sizeof(Type));
  node->type->ty = INT;
  node->type->size = 4;
  return node;
}

void program() {
  int i = 0;
  while (!at_eof())
    code[i++] = function();
  code[i] = NULL;
}

Type *type() {
  Type *ty = calloc(1, sizeof(Type));

  if (consume_token(TK_INT)) {
    ty->ty = INT;
    ty->size = 4;
  } else {
    return NULL;
  }

  while (consume("*")) {
    Type *ptr = calloc(1, sizeof(Type));
    ptr->ty = PTR;
    ptr->size = 8;
    ptr->ptr_to = ty;
    ty = ptr;
  }

  return ty;
}

Node *function() {
  Type *ty = type();
  if (!ty) error_at(token->str, "type expected");

  Node *node = new_node(ND_FUNC, NULL, NULL);
  Token *tok = consume_ident();
  if (!tok) error_at(token->str, "identifier expected");

  if (!consume("(")) {
    LVar *lvar = calloc(1, sizeof(LVar));
    lvar->name = tok->str;
    lvar->len = tok->len;
    lvar->type = ty;
    if (ty->ty == ARRAY)
      lvar->offset = ty->size * ty->array_size;
    else
      lvar->offset = ty->size;

    char *name = malloc(sizeof(char) * 64);
    strncpy(name, lvar->name, lvar->len);
    name[lvar->len] = '\x0';
    map_put(globals, name, lvar);

    Node *node = new_node(ND_DECLARE_GVAR, NULL, NULL);
    node->name = tok->str;
    node->len = tok->len;
    node->offset = lvar->offset;
    expect(";");
    return node;
  }

  node = new_node(ND_FUNC, NULL, NULL);
  node->name = tok->str;
  node->len = tok->len;

  Node *cur;
  cur = node;

  while (ty = type()) {
    tok = consume_ident();
    Node *arg = new_node(ND_LVAR, NULL, NULL);
    LVar *lvar = find_lvar(tok);

    if (lvar) {
      arg->offset = lvar->offset;

    } else {
      lvar = calloc(1, sizeof(LVar));
      lvar->next = locals;
      lvar->name = tok->str;
      lvar->len = tok->len;
      lvar->offset = locals->offset + 8;
      lvar->type = ty;
      arg->offset = lvar->offset;
      arg->type = lvar->type;
      locals = lvar;
    }

    if (cur == node) {
      cur->child = arg;
      cur = cur->child;
    } else {
      cur->next = arg;
      cur = cur->next;
    }

    if (!consume(",")) break;
  }

  expect(")");

  if (!consume("{")) error_at(token->str, "block expected");
  Node *block = new_node(ND_BLOCK, NULL, NULL);

  if (!consume("}")) {
    block->child = stmt();
    cur = block->child;
  }

  while (!consume("}")) {
    cur->next = stmt();
    cur = cur->next;
  }

  node->next = block;
  return node;
}

Node *stmt() {
  if (consume_token(TK_RETURN)) {
    Node *node = new_node(ND_RETURN, expr(), NULL);
    expect(";");
    return node;
  }

  if (consume_token(TK_IF)) {
    Node *node = new_node(ND_IF, NULL, NULL);
    expect("(");
    node->cond = expr();
    expect(")");
    node->body = stmt();
    if (consume_token(TK_ELSE)) {
      node->els = stmt();
    }
    return node;
  }

  if (consume_token(TK_FOR)) {
    Node *node = new_node(ND_FOR, NULL, NULL);
    expect("(");
    if (!consume(";")) {
      node->init = expr();
      expect(";");
    }
    if (!consume(";")) {
      node->cond = expr();
      expect(";");
    }
    if (!consume(")")) {
      node->inc = expr();
      expect(")");
    }
    node->body = stmt();
    return node;
  }

  if (consume_token(TK_WHILE)) {
    Node *node = new_node(ND_WHILE, NULL, NULL);
    expect("(");
    node->cond = expr();
    expect(")");
    node->body = stmt();
    return node;
  }

  if (consume("{")) {
    Node *node = new_node(ND_BLOCK, NULL, NULL);

    Node *cur;
    if (!consume("}")) {
      node->child = stmt();
      cur = node->child;
    }

    while (!consume("}")) {
      cur->next = stmt();
      cur = cur->next;
    }
    return node;
  }

  Type *ty;
  if (ty = type()) {
    Token *tok = consume_ident();

    if (consume("[")) {
      Type *ary = calloc(1, sizeof(Type));
      ary->ty = ARRAY;
      ary->size = 8;
      ary->array_size = expect_number();
      ary->ptr_to = ty;
      ty = ary;
      expect("]");
    }

    expect(";");

    LVar *lvar = find_lvar(tok);
    if (lvar) error_at(token->str, "already declared");

    lvar = calloc(1, sizeof(LVar));
    lvar->next = locals;
    lvar->name = tok->str;
    lvar->len = tok->len;
    if (ty->ty == ARRAY)
      lvar->offset = locals->offset + 8 * ty->array_size;
    else
      lvar->offset = locals->offset + 8;
    lvar->type = ty;
    locals = lvar;

    Node *node = new_node(ND_DECLARE, NULL, NULL);
    return node;
  }

  Node *node = expr();
  expect(";");
  return node;
}

Node *expr() {
  return assign();
}

Node *assign() {
  Node *node = equality();
  if (consume("="))
    node = new_expr(ND_ASSIGN, node, assign());
  return node;
}

Node *equality() {
  Node *node = relational();
  while (true) {
    if (consume("=="))
      node = new_expr(ND_EQ, node, relational());
    else if (consume("!="))
      node = new_expr(ND_NE, node, relational());
    else
      return node;
  }
}

Node *relational() {
  Node *node = add();
  while (true) {
    if (consume("<="))
      node = new_expr(ND_LE, node, add());
    else if (consume(">="))
      node = new_expr(ND_LE, add(), node);
    else if (consume("<"))
      node = new_expr(ND_LT, node, add());
    else if (consume(">"))
      node = new_expr(ND_LT, add(), node);
    else
      return node;
  }
}

Node *add() {
  Node *node = mul();
  while (true) {
    if (consume("+")) {
      node = new_expr(ND_ADD, node, mul());
      if (node->lhs->type->ty == PTR) {
        if (node->rhs->kind != ND_NUM) error_at(token->str, "not supported");
        node->type = node->lhs->type;
        node->rhs->val *= node->type->ptr_to->size;
      }
      if (node->rhs->type->ty == PTR) {
        if (node->lhs->kind != ND_NUM) error_at(token->str, "not supported");
        node->type = node->rhs->type;
        node->lhs->val *= node->type->ptr_to->size;
      }
    } else if (consume("-")) {
      node = new_expr(ND_SUB, node, mul());
      if (node->lhs->type->ty == PTR) {
        if (node->rhs->kind != ND_NUM) error_at(token->str, "not supported");
        node->type = node->lhs->type;
        node->rhs->val *= node->type->ptr_to->size;
      }
    } else {
      return node;
    }
  }
}

Node *mul() {
  Node *node = unary();
  while (true) {
    if (consume("*"))
      node = new_expr(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_expr(ND_DIV, node, unary());
    else
      return node;
  }
}

Node *unary() {
  if (consume("+"))
    return primary();

  if (consume("-"))
    return new_expr(ND_SUB, new_node_num(0), primary());

  if (consume("*"))
    return new_expr(ND_DEREF, unary(), NULL);

  if (consume("&")) {
    Node *node = unary();
    if (node->type->ty == ARRAY)
      return node;
    return new_expr(ND_ADDR, node, NULL);
  }

  if (consume_token(TK_SIZEOF)) {
    Node *node = unary();
    if (node->type->ty == ARRAY)
      new_node_num(node->type->size * node->type->array_size);
    return new_node_num(node->type->size);
  }

  Node *node = primary();
  if (node->type->ty == ARRAY) {
    node->type->ty = PTR;
    node->type->size = 8;
  }
  return node;
}

Node *primary() {
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  Token *tok = consume_ident();
  if (tok) {
    Node *node = calloc(1, sizeof(Node));
    char *name = tok->str;
    int len = tok->len;

    if (consume("(")) {
      node->kind = ND_CALL;
      node->name = name;
      node->len = len;
      node->type = NULL;  // TODO

      if (consume(")")) return node;
      node->child = expr();
      Node *cur = node->child;

      while (!consume(")")) {
        expect(",");
        cur->next = expr();
        cur = cur->next;
      }

      return node;
    }

    LVar *lvar = find_lvar(tok);
    if (lvar) {
      node->kind = ND_LVAR;
    } else {
      char name[64];
      strncpy(name, tok->str, tok->len);
      name[tok->len] = '\x0';
      lvar = map_get(globals, name);
      if (!lvar) error_at(tok->str, "not declared");
      node->kind = ND_GVAR;
    }

    node->name = lvar->name;
    node->len = lvar->len;

    if (consume("[")) {
      node->offset = lvar->offset + expect_number() * lvar->type->size;
      node->type = lvar->type->ptr_to;
      expect("]");
    } else {
      node->offset = lvar->offset;
      node->type = lvar->type;
    }
    return node;
  }

  return new_node_num(expect_number());
}
