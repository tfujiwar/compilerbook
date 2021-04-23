#include "mycc.h"

Scope *new_scope(Scope *parent) {
  Scope *scope = calloc(1, sizeof(Scope));
  scope->parent = parent;
  scope->vars = new_map();
  return scope;
}

bool consume_token(int kind) {
  if (token->kind != kind)
    return false;

  token = token->next;
  return true;
}

bool consume(char *op) {
  if ((token->kind != TK_RESERVED) || strcmp(token->str, op) != 0)
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

Token *consume_string() {
  if (token->kind != TK_STRING)
    return NULL;

  Token *tok = token;
  token = token->next;
  return tok;
}

LVar *find_lvar(Token *tok) {
  for (LVar *var = locals; var; var = var->next) {
    if (var->name && strcmp(tok->str, var->name) == 0) return var;
  }
  return NULL;
}

bool expect_token(int kind) {
  if (token->kind != kind)
    error_at(token->at, "expected %d, but got %d", kind, token->kind);
  token = token->next;
}

void expect(char *op) {
  if ((token->kind != TK_RESERVED) || strcmp(token->str, op) != 0)
    error_at(token->at, "not a '%s'", op);
  token = token->next;
}

int expect_number() {
  if (token->kind != TK_NUM)
    error_at(token->at, "not a 'number'");
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() {
  return token->kind == TK_EOF;
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
    code[i++] = function();
  code[i] = NULL;
}

Type *type() {
  Type *ty = calloc(1, sizeof(Type));

  if (consume_token(TK_CHAR)) {
    ty->ty = CHAR;
    ty->size = 1;
  } else if (consume_token(TK_INT)) {
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
  if (!ty) error_at(token->at, "type expected");

  Token *ident = consume_ident();
  if (!ident) error_at(ident->at, "identifier expected");

  // Global variable
  if (!consume("(")) {
    if (consume("[")) {
      Type *ary = calloc(1, sizeof(Type));
      ary->ty = ARRAY;
      ary->size = 8;
      ary->array_size = expect_number();
      ary->ptr_to = ty;
      ty = ary;
      expect("]");
    }

    LVar *lvar = calloc(1, sizeof(LVar));
    lvar->name = ident->str;
    lvar->type = ty;
    if (ty->ty == ARRAY)
      lvar->offset = ty->size * ty->array_size;
    else
      lvar->offset = ty->size;

    map_put(global->vars, lvar->name, lvar);

    Node *node = new_node(ND_DECLARE_GVAR, NULL, NULL);
    node->lvar = lvar;
    expect(";");
    return node;
  }

  // Function
  Node *node = new_node(ND_FUNC, NULL, NULL);
  node->name = ident->str;
  map_put(functions, node->name, ty);

  // Function args
  Node *cur = node;
  while (ty = type()) {
    Token *ident = consume_ident();
    Node *arg = new_node(ND_LVAR, NULL, NULL);
    LVar *lvar = find_lvar(ident);

    if (!lvar) {
      lvar = calloc(1, sizeof(LVar));
      lvar->next = locals;
      lvar->name = ident->str;
      lvar->offset = locals->offset + 8;
      lvar->type = ty;
      locals = lvar;
    }

    arg->lvar = lvar;

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

  // Function body
  expect("{");
  Node *block = new_node(ND_BLOCK, NULL, NULL);

  cur = block;
  while (!consume("}")) {
    if (cur == block) {
      cur->child = stmt();
      cur = cur->child;
    } else {
      cur->next = stmt();
      cur = cur->next;
    }
  }

  node->body = block;
  return node;
}

Node *stmt() {
  // Return statement
  if (consume_token(TK_RETURN)) {
    Node *node = new_node(ND_RETURN, expr(), NULL);
    expect(";");
    return node;
  }

  // If statement
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

  // For statement
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

  // While statement
  if (consume_token(TK_WHILE)) {
    Node *node = new_node(ND_WHILE, NULL, NULL);
    expect("(");
    node->cond = expr();
    expect(")");
    node->body = stmt();
    return node;
  }

  // Block
  if (consume("{")) {
    Node *node = new_node(ND_BLOCK, NULL, NULL);
    Node *cur = node;
    while (!consume("}")) {
      if (cur == node) {
        cur->child = stmt();
        cur = cur->child;
      } else {
        cur->next = stmt();
        cur = cur->next;
      }
    }
    return node;
  }

  // Declare local variable
  Type *ty;
  if (ty = type()) {
    Token *ident = consume_ident();

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

    LVar *lvar = find_lvar(ident);
    if (lvar) error_at(token->at, "already declared");

    lvar = calloc(1, sizeof(LVar));
    lvar->next = locals;
    lvar->name = ident->str;
    if (ty->ty == ARRAY)
      lvar->offset = locals->offset + ty->size * ty->array_size;
    else
      lvar->offset = locals->offset + ty->size;
    lvar->type = ty;
    locals = lvar;

    Node *node = new_node(ND_DECLARE, NULL, NULL);
    return node;
  }

  // Expression
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
    if (consume("+")) {
      node = new_node(ND_ADD, node, mul());
    } else if (consume("-")) {
      node = new_node(ND_SUB, node, mul());
    } else {
      return node;
    }
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

  if (consume("*"))
    return new_node(ND_DEREF, unary(), NULL);

  if (consume("&"))
    return new_node(ND_ADDR, unary(), NULL);

  if (consume_token(TK_SIZEOF))
    return new_node(ND_SIZEOF, expr(), NULL);

  Node *node = primary();
  return node;
}

Node *primary() {
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  Token *ident = consume_ident();
  if (ident) {
    Node *node = calloc(1, sizeof(Node));
    char *name = ident->str;

    // Function call
    if (consume("(")) {
      node->kind = ND_CALL;
      node->name = name;

      Type *func = map_get(functions, name);
      node->type = func;

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

    // Local or global variable
    LVar *lvar = find_lvar(ident);
    if (lvar) {
      node->kind = ND_LVAR;
      node->lvar = lvar;
    } else {
      lvar = map_get(global->vars, ident->str);
      if (!lvar) error_at(ident->at, "not declared");
      node->kind = ND_GVAR;
      node->lvar = lvar;
    }

    if (consume("[")) {
      node = new_node(ND_DEREF, new_node(ND_ADD, node, expr()), NULL);
      expect("]");
    }
    return node;
  }

  Token *str;
  if (str = consume_string()) {
    Node *node = new_node(ND_STRING, NULL, NULL);
    node->val = strings->keys->len;
    map_put(strings, str->str, NULL);
    return node;
  }

  return new_node_num(expect_number());
}
