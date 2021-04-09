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

    if (memcmp(p, "==", 2) == 0 || memcmp(p, "!=", 2) == 0 || memcmp(p, "<=", 2) == 0 || memcmp(p, ">=", 2) == 0) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' || *p == '<' || *p == '>' || *p == ';' || *p == '=') {
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

Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

void program() {
  int i = 0;
  while (!at_eof())
    code[i++] = stmt();
  code[i] = NULL;
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
    node = new_node(ND_ASSIGN, node, assign());
  return node;
}

Node *equality() {
  Node *node = relational();
  while (true) {
    if (consume("=="))
      node = new_node(ND_EQ, node, relational());
    else if (consume("!="))
      node = new_node(ND_NE, node, relational());
    else
      return node;
  }
}

Node *relational() {
  Node *node = add();
  while (true) {
    if (consume("<="))
      node = new_node(ND_LE, node, add());
    else if (consume(">="))
      node = new_node(ND_LE, add(), node);
    else if (consume("<"))
      node = new_node(ND_LT, node, add());
    else if (consume(">"))
      node = new_node(ND_LT, add(), node);
    else
      return node;
  }
}

Node *add() {
  Node *node = mul();
  while (true) {
    if (consume("+"))
      node = new_node(ND_ADD, node, mul());
    else if (consume("-"))
      node = new_node(ND_SUB, node, mul());
    else
      return node;
  }
}

Node *mul() {
  Node *node = unary();
  while (true) {
    if (consume("*"))
      node = new_node(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_node(ND_DIV, node, unary());
    else
      return node;
  }
}

Node *unary() {
  if (consume("+"))
    return primary();
  if (consume("-"))
    return new_node(ND_SUB, new_node_num(0), primary());
  else
    return primary();
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
    node->kind = ND_LVAR;

    LVar *lvar = find_lvar(tok);
    if (lvar) {
      node->offset = lvar->offset;
    } else {
      lvar = calloc(1, sizeof(LVar));
      lvar->next = locals;
      lvar->name = tok->str;
      lvar->len = tok->len;
      lvar->offset = locals->offset + 8;
      node->offset = lvar->offset;
      locals = lvar;
    }

    return node;
  }

  return new_node_num(expect_number());
}
