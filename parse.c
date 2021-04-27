#include "mycc.h"

int offset = 0;

Scope *new_scope(Scope *parent) {
  Scope *scope = calloc(1, sizeof(Scope));
  scope->parent = parent;
  scope->vars = new_map();
  return scope;
}

LVar *new_var(char *name) {
  LVar *lvar = calloc(1, sizeof(LVar));
  lvar->name = name;
  map_put(scope->vars, name, lvar);
  return lvar;
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
  if (token->kind != TK_IDENT || !is_ident_char(token->str[0]))
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

Token *consume_num() {
  if (token->kind != TK_NUM)
    return NULL;

  Token *tok = token;
  token = token->next;
  return tok;
}

LVar *find_lvar(char *name) {
  LVar *lvar;
  Scope *sc = scope;
  while (sc) {
    if (lvar = map_get(sc->vars, name)) return lvar;
    sc = sc->parent;
  }
  return NULL;
}

LVar *find_lvar_in_scope(char *name) {
  return map_get(scope->vars, name);
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

Node *new_node_string(char* str) {
  Node *node = new_node(ND_STRING, NULL, NULL);
  node->val = strings->keys->len;
  map_put(strings, str, NULL);
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
      ary->ptr_to = ty;

      Token *tok;
      if (tok = consume_num()) {
        ary->array_size = tok->val;
        ary->size = ary->ptr_to->size * tok->val;
      }

      ty = ary;
      expect("]");
    }

    LVar *lvar = new_var(ident->str);
    lvar->is_global = true;
    lvar->type = ty;

    // Initialize variable
    if (consume("=")) {

      // Initialize with array literal
      if (consume("{")) {
        if (lvar->type->ty != ARRAY) error_at(token->at, "not an array");

        Node *node = new_node(ND_DECLARE_GVAR, NULL, NULL);
        node->lvar = lvar;

        int index = 0;
        Node *cur = node;

        while (true) {
          Node *child = expr();
          if (cur == node) {
            cur->rhs = child;
            cur = child;
          } else {
            cur->next = child;
            cur = child;
          }
          index++;
          if (!consume(",")) break;
        }

        if (node->lvar->type->array_size == 0) {
          node->lvar->type->array_size = index;
          node->lvar->type->size = node->lvar->type->ptr_to->size * index;
        }

        expect("}");
        expect(";");
        return node;
      }

      // Initialize with string literal
      Token *str;
      if ((str = consume_string()) && lvar->type->ty == ARRAY) {
        Node *node = new_node(ND_DECLARE_GVAR, NULL, NULL);
        node->lvar = lvar;

        Node *cur = node;

        for (int i = 0; i < strlen(str->str) + 1; i++) {
          Node *child = new_node_num(str->str[i]);
          if (cur == node) {
            cur->rhs = child;
            cur = child;
          } else {
            cur->next = child;
            cur = child;
          }
        }

        if (node->lvar->type->array_size == 0) {
          node->lvar->type->array_size = strlen(str->str) + 1;
          node->lvar->type->size = node->lvar->type->ptr_to->size * (strlen(str->str) + 1);
        }

        expect(";");
        return node;
      }

      // Initialize with other expression
      Node *node = new_node(ND_DECLARE_GVAR, NULL, NULL);
      node->lvar = lvar;
      node->rhs = expr();
      expect(";");
      return node;
    }

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
  scope = new_scope(scope);
  offset = 0;

  Node *cur = node;
  while (ty = type()) {
    Token *ident = consume_ident();
    Node *arg = new_node(ND_LVAR, NULL, NULL);
    LVar *lvar = new_var(ident->str);
    offset += ty->size;
    lvar->is_global = false;
    lvar->next = locals;
    lvar->offset = offset;
    lvar->type = ty;
    locals = lvar;

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

  scope = scope->parent;
  node->val = offset;

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
    scope = new_scope(scope);
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
    scope = scope->parent;

    return node;
  }

  // Declare local variable
  Type *ty;
  if (ty = type()) {
    Token *ident = consume_ident();

    if (consume("[")) {
      Type *ary = calloc(1, sizeof(Type));
      ary->ty = ARRAY;
      ary->ptr_to = ty;

      Token *tok;
      if (tok = consume_num()) {
        ary->array_size = tok->val;
        ary->size = ary->ptr_to->size * tok->val;
      }

      ty = ary;
      expect("]");
    }

    LVar *lvar = find_lvar_in_scope(ident->str);
    if (lvar) error_at(ident->at, "already declared");

    lvar = new_var(ident->str);
    lvar->is_global = false;
    lvar->next = locals;

    if (ty->ty == ARRAY) {
      offset += ty->size * ty->array_size;
      lvar->offset = offset;
    } else {
      offset += ty->size;
      lvar->offset = offset;
    }

    lvar->type = ty;
    locals = lvar;

    // Initialize variable
    if (consume("=")) {

      // Initialize with array literal
      if (consume("{")) {
        if (lvar->type->ty != ARRAY) error_at(token->at, "not an array");

        Node *node = new_node(ND_DECLARE, NULL, NULL);
        node->lvar = lvar;

        int index = 0;
        Node *cur = node;

        while (true) {
          Node *nd = new_node(ND_LVAR, NULL, NULL);
          nd->lvar = lvar;
          Node *lhs = new_node(ND_DEREF, new_node(ND_ADD, nd, new_node_num(index++)), NULL);
          Node *child = new_node(ND_ASSIGN, lhs, expr());

          if (cur == node) {
            cur->child = child;
            cur = child;
          } else {
            cur->next = child;
            cur = child;
          }

          if (!consume(",")) break;
        }

        if (node->lvar->type->array_size == 0) {
          node->lvar->type->array_size = index;
          node->lvar->type->size = node->lvar->type->ptr_to->size * index;
        }

        offset += node->lvar->type->size;
        lvar->offset = offset;

        for (;index <= node->lvar->type->array_size; index++) {
          Node *nd = new_node(ND_LVAR, NULL, NULL);
          nd->lvar = lvar;
          Node *lhs = new_node(ND_DEREF, new_node(ND_ADD, nd, new_node_num(index)), NULL);
          Node *child = new_node(ND_ASSIGN, lhs, new_node_num(0));

          if (cur == node) {
            cur->child = child;
            cur = child;
          } else {
            cur->next = child;
            cur = child;
          }
        }

        expect("}");
        expect(";");
        return node;
      }

      // Initialize with string literal
      Token *str;
      if ((str = consume_string()) && lvar->type->ty == ARRAY) {
        Node *node = new_node(ND_DECLARE, NULL, NULL);
        node->lvar = lvar;

        int index = 0;
        Node *cur = node;

        for (int i = 0; i < strlen(str->str); i++) {
          Node *nd = new_node(ND_LVAR, NULL, NULL);
          nd->lvar = lvar;
          Node *lhs = new_node(ND_DEREF, new_node(ND_ADD, nd, new_node_num(index++)), NULL);
          Node *child = new_node(ND_ASSIGN, lhs, new_node_num(str->str[i]));

          if (cur == node) {
            cur->child = child;
            cur = child;
          } else {
            cur->next = child;
            cur = child;
          }
        }

        if (node->lvar->type->array_size == 0) {
          node->lvar->type->array_size = index + 1;
          node->lvar->type->size = node->lvar->type->ptr_to->size * (index + 1);
        }

        offset += node->lvar->type->size;
        lvar->offset = offset;

        for (;index < node->lvar->type->array_size; index++) {
          Node *nd = new_node(ND_LVAR, NULL, NULL);
          nd->lvar = lvar;
          Node *lhs = new_node(ND_DEREF, new_node(ND_ADD, nd, new_node_num(index)), NULL);
          Node *child = new_node(ND_ASSIGN, lhs, new_node_num(0));

          if (cur == node) {
            cur->child = child;
            cur = child;
          } else {
            cur->next = child;
            cur = child;
          }
        }
        expect(";");
        return node;
      }

      // Initialize with other expression
      Node *lhs = new_node(ND_LVAR, NULL, NULL);
      lhs->lvar = lvar;

      Node *node = new_node(ND_ASSIGN, lhs, expr());;
      expect(";");
      return node;

    } else if (lvar->type->ty == ARRAY && lvar->type->array_size == 0) {
      error_at(token->at, "array size or initialization is needed");
    }

    Node *node = new_node(ND_DECLARE, NULL, NULL);
    expect(";");
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
  Node *node = conditional();
  if (consume("="))
    return new_node(ND_ASSIGN, node, assign());
  else if (consume("+="))
    return new_node(ND_ASSIGN, node, new_node(ND_ADD, node, assign()));
  else if (consume("-="))
    return new_node(ND_ASSIGN, node, new_node(ND_SUB, node, assign()));
  else if (consume("*="))
    return new_node(ND_ASSIGN, node, new_node(ND_MUL, node, assign()));
  else if (consume("/="))
    return new_node(ND_ASSIGN, node, new_node(ND_DIV, node, assign()));
  else if (consume("%="))
    return new_node(ND_ASSIGN, node, new_node(ND_MOD, node, assign()));
  else if (consume("<<="))
    return new_node(ND_ASSIGN, node, new_node(ND_SHIFT_LEFT, node, assign()));
  else if (consume(">>="))
    return new_node(ND_ASSIGN, node, new_node(ND_SHIFT_RIGHT, node, assign()));
  else if (consume("&="))
    return new_node(ND_ASSIGN, node, new_node(ND_BITWISE_AND, node, assign()));
  else if (consume("^="))
    return new_node(ND_ASSIGN, node, new_node(ND_BITWISE_XOR, node, assign()));
  else if (consume("|="))
    return new_node(ND_ASSIGN, node, new_node(ND_BITWISE_OR, node, assign()));
  else
    return node;
}

Node *conditional() {
  Node *node = logical_or();
  if (consume("?")) {
    Node *nd = new_node(ND_CONDITIONAL, NULL, NULL);
    nd->cond = node;
    nd->body = conditional();
    expect(":");
    nd->els = conditional();
    return nd;
  } else {
    return node;
  }
}

Node *logical_or() {
  Node *node = logical_and();
  while (true) {
    if (consume("||"))
      node = new_node(ND_LOGICAL_OR, node, logical_and());
    else
      return node;
  }
}

Node *logical_and() {
  Node *node = bitwise_or();
  while (true) {
    if (consume("&&"))
      node = new_node(ND_LOGICAL_AND, node, bitwise_or());
    else
      return node;
  }
}

Node *bitwise_or() {
  Node *node = bitwise_xor();
  while (true) {
    if (consume("|"))
      node = new_node(ND_BITWISE_OR, node, bitwise_xor());
    else
      return node;
  }
}

Node *bitwise_xor() {
  Node *node = bitwise_and();
  while (true) {
    if (consume("^"))
      node = new_node(ND_BITWISE_XOR, node, bitwise_and());
    else
      return node;
  }
}

Node *bitwise_and() {
  Node *node = equality();
  while (true) {
    if (consume("&"))
      node = new_node(ND_BITWISE_AND, node, equality());
    else
      return node;
  }
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
  Node *node = shift();
  while (true) {
    if (consume("<="))
      node = new_node(ND_LE, node, shift());
    else if (consume(">="))
      node = new_node(ND_LE, shift(), node);
    else if (consume("<"))
      node = new_node(ND_LT, node, shift());
    else if (consume(">"))
      node = new_node(ND_LT, shift(), node);
    else
      return node;
  }
}

Node *shift() {
  Node *node = add();
  while (true) {
    if (consume("<<"))
      node = new_node(ND_SHIFT_LEFT, node, add());
    else if (consume(">>"))
      node = new_node(ND_SHIFT_RIGHT, node, add());
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
  Node *node = unary_left();
  while (true) {
    if (consume("*"))
      node = new_node(ND_MUL, node, unary_left());
    else if (consume("/"))
      node = new_node(ND_DIV, node, unary_left());
    else if (consume("%"))
      node = new_node(ND_MOD, node, unary_left());
    else
      return node;
  }
}

Node *unary_left() {
  if (consume("+"))
    return unary_left();

  if (consume("-"))
    return new_node(ND_SUB, new_node_num(0), unary_left());

  if (consume("*"))
    return new_node(ND_DEREF, unary_left(), NULL);

  if (consume("&"))
    return new_node(ND_ADDR, unary_left(), NULL);

  if (consume_token(TK_SIZEOF))
    return new_node(ND_SIZEOF, unary_left(), NULL);

  if (consume("++")) {
    Node *nd = unary_left();
    return new_node(ND_ASSIGN, nd, new_node(ND_ADD, nd, new_node_num(1)));
  }

  if (consume("--")) {
    Node *nd = unary_left();
    return new_node(ND_ASSIGN, nd, new_node(ND_SUB, nd, new_node_num(1)));
  }

  if (consume("!"))
    return new_node(ND_EQ, unary_left(), new_node_num(0));

  if (consume("~"))
    return new_node(ND_BITWISE_NOT, unary_left(), NULL);

  Node *node = unary_right();
  return node;
}

Node *unary_right() {
  Node *node = primary();
  while (true) {
    if (consume("[")) {
      node = new_node(ND_DEREF, new_node(ND_ADD, node, unary_right()), NULL);
      expect("]");

    } else if (consume("(")) {
      node = new_node(ND_CALL, node, NULL);
      if (consume(")")) continue;

      node->child = expr();
      Node *cur = node->child;

      while (!consume(")")) {
        expect(",");
        cur->next = expr();
        cur = cur->next;
      }

    } else {
      return node;
    }
  }
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

    // Function name
    Type *func = map_get(functions, name);
    if (func) {
      node->kind = ND_FUNC_NAME;
      node->name = name;
      node->type = func;
      return node;
    }

    // Local or global variable
    LVar *lvar = find_lvar(ident->str);
    if (!lvar) error_at(ident->at, "not declared");

    node->lvar = lvar;
    node->kind = lvar->is_global ? ND_GVAR : ND_LVAR;
    return node;
  }

  Token *str;
  if (str = consume_string()) {
    return new_node_string(str->str);
  }

  return new_node_num(expect_number());
}
