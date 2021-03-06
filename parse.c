#include "mycc.h"

int func_offset = 0;
int struct_label = 0;
int enum_label = 0;

Scope *new_scope(Scope *parent) {
  Scope *scope = calloc(1, sizeof(Scope));
  scope->parent = parent;
  scope->vars = new_map();
  scope->types = new_map();
  scope->structs = new_map();
  scope->enums = new_map();
  scope->enum_consts = new_map();
  return scope;
}

SwitchScope *new_switch_scope(SwitchScope *parent, Node *node) {
  SwitchScope *scope = calloc(1, sizeof(SwitchScope));
  scope->parent = parent;
  scope->node = node;
  return scope;
}

BreakScope *new_break_scope(BreakScope *parent, Node *node) {
  BreakScope *scope = calloc(1, sizeof(BreakScope));
  scope->parent = parent;
  scope->node = node;
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

Node *parse_typedef() {
  if (consume_token(TK_TYPEDEF)) {
    Type *ty = consume_type();
    Token *ident = consume_ident();
    if (!ident) error_at_token(token, "identifier expected");
    expect(";");

    map_put(scope->types, ident->str, ty);
    return new_node(ND_DECLARE, NULL, NULL);
  }
  return NULL;
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

Type *find_type(char *name) {
  Type *type;
  Scope *sc = scope;
  while (sc) {
    if (type = map_get(sc->types, name)) return type;
    sc = sc->parent;
  }
  return NULL;
}

Type *find_struct(char *name) {
  Type *type;
  Scope *sc = scope;
  while (sc) {
    if (type = map_get(sc->structs, name)) return type;
    sc = sc->parent;
  }
  return NULL;
}

Type *find_enum(char *name) {
  Type *type;
  Scope *sc = scope;
  while (sc) {
    if (type = map_get(sc->enums, name)) return type;
    sc = sc->parent;
  }
  return NULL;
}

int *find_enum_consts(char *name) {
  int *val;
  Scope *sc = scope;
  while (sc) {
    if (val = map_get(sc->enum_consts, name)) return val;
    sc = sc->parent;
  }
  return NULL;
}

bool expect_token(int kind) {
  if (token->kind != kind) {
    char str[256];
    sprintf(str, "expected %d, but got %d", kind, token->kind);
    error_at_token(token, str);
  }
  token = token->next;
}

void expect(char *op) {
  if ((token->kind != TK_RESERVED) || strcmp(token->str, op) != 0) {
    char str[256];
    sprintf(str, "not a '%s'", op);
    error_at_token(token, str);
  }
  token = token->next;
}

int expect_number() {
  if (token->kind != TK_NUM)
    error_at_token(token, "not a 'number'");
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
  node->name = str;
  map_put(strings, str, NULL);
  return node;
}

Function *new_function(char *name, Type *type) {
  Function *func = calloc(1, sizeof(Function));
  func->name = name;
  func->return_type = type;
  func->args = new_vec();
  func->arg_types = new_vec();
  return func;
}

Struct *new_struct(char *name) {
  Struct *strct = calloc(1, sizeof(Struct));
  strct->name = name;
  strct->member = new_map();
  return strct;
}

Member *new_member(char *name, Type *type, int offset) {
  Member *member = calloc(1, sizeof(Member));
  member->name = name;
  member->type = type;
  member->offset = offset;
  return member;
}

Enum *new_enum(char *name) {
  Enum *enm = calloc(1, sizeof(Enum));
  enm->name = name;
  return enm;
}

void program() {
  int i = 0;
  while (!at_eof())
    code[i++] = function();
  code[i] = NULL;
}

Type *consume_type() {
  Type *ty = NULL;
  Token *ident = NULL;
  Token *tok = token;

  if (consume_token(TK_STRUCT)) {
    ident = consume_ident();
    if (!ident || !(ty = find_struct(ident->str)) || ty->strct->is_proto) {
      char* st_name;
      if (ident) {
        st_name = ident->str;
      } else {
        st_name = calloc(1, sizeof(char) * 10);
        sprintf(st_name, "struct%03d", struct_label++);
      }

      if (!ty) {
        ty = calloc(1, sizeof(Type));
        ty->ty = STRUCT;
        ty->strct = new_struct(st_name);
        ty->strct->is_proto = true;
        map_put(scope->structs, ty->strct->name, ty);
      }

      if (consume("{")) {
        int offset = 0;

        while (!consume("}")) {
          Type *mem_ty = consume_type();
          Token *mem = consume_ident();
          Member *member = new_member(mem->str, mem_ty, offset);
          map_put(ty->strct->member, member->name, member);
          offset += mem_ty->size;
          expect(";");
        }

        ty->size = (offset + 3) / 4 * 4;
        ty->strct->is_proto = false;
      }
    }

  } else if (consume_token(TK_ENUM)) {
    ident = consume_ident();
    if (!ident || !(ty = find_enum(ident->str))) {
      char *en_name;
      if (ident) {
        en_name = ident->str;
      } else {
        en_name = calloc(1, sizeof(char) * 8);
        sprintf(en_name, "enum%03d", enum_label++);
      }

      expect("{");
      Type *en_type = calloc(1, sizeof(Type));
      en_type->ty = ENUM;
      en_type->enm = new_enum(en_name);
      en_type->size = 4;
      map_put(scope->enums, en_type->enm->name, en_type);

      int num = 0;

      while (!consume("}")) {
        Token *id = consume_ident();
        if (!id) error_at_token(token, "identifier expected");

        if (consume("=")) {
          Token *specified_num;
          if (specified_num = consume_num()) {
            num = specified_num->val;
          }
        }

        int *val = calloc(1, sizeof(int));
        *val = num;
        map_put(scope->enum_consts, id->str, val);

        if (!consume(",")) {
          expect("}");
          break;
        }
        num++;
      }

      ty = en_type;
    }

  } else if (ident = consume_ident()) {
    ty = find_type(ident->str);
    if (!ty) {
      token = tok;
      return NULL;
    }

  } else {
    ty = calloc(1, sizeof(Type));
    if (consume_token(TK_VOID)) {
      ty->ty = VOID;
      ty->size = 1;
    } else if (consume_token(TK_CHAR)) {
      ty->ty = CHAR;
      ty->size = 1;
    } else if (consume_token(TK_INT)) {
      ty->ty = INT;
      ty->size = 4;
    } else if (consume_token(TK_ELLIPSIS)) {
      ty->ty = ELLIPSIS;
    } else {
      return NULL;
    }
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
  bool is_extern = consume_token(TK_EXTERN);

  // Struct Definition
  Node *node;
  if (node = parse_typedef()) return node;

  Type *ty = consume_type();
  if (!ty) error_at_token(token, "type expected");
  if (ty->ty == STRUCT && consume(";")) return new_node(ND_DECLARE, NULL, NULL);
  if (ty->ty == ENUM && consume(";")) return new_node(ND_DECLARE, NULL, NULL);

  Token *ident = consume_ident();
  if (!ident) error_at_token(token, "identifier expected");

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
    lvar->is_extern = is_extern;

    // Initialize variable
    if (consume("=")) {

      // Initialize with array literal
      if (consume("{")) {
        if (lvar->type->ty != ARRAY) error_at_token(token, "not an array");

        Node *node = new_node(ND_DECLARE_GVAR, NULL, NULL);
        node->lvar = lvar;

        int index = 0;
        Node *cur = node;

        while (true) {
          Node *child = assign();
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
        str->str = convert_escaped_str(str->str);

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
      node->rhs = assign();
      expect(";");
      return node;
    }

    Node *node = new_node(ND_DECLARE_GVAR, NULL, NULL);
    node->lvar = lvar;
    expect(";");
    return node;
  }

  // Function
  Function *func = map_get(functions, ident->str);
  if (func && !func->is_proto) error_at_token(ident, "function already defined");
  if (!func) {
    func = new_function(ident->str, ty);
    map_put(functions, func->name, func);
  }

  // Function args
  func->is_proto = false;
  Vector *arg_names = new_vec();
  Vector *arg_types = new_vec();
  Type *arg_ty;

  while (arg_ty = consume_type()) {
    vec_push(arg_types, arg_ty);

    Token *ident = consume_ident();
    if (ident) vec_push(arg_names, ident->str);
    else if (arg_ty->ty == ELLIPSIS) vec_push(arg_names, "ELLIPSIS");
    else func->is_proto = true;

    if (!consume(",")) break;
  }
  expect(")");

  // Function body
  if (consume("{")) {
    if (func->is_proto) error_at_token(ident, "argument name missing");

    if (func->arg_types->len) {
      if (arg_types->len != func->arg_types->len)
        error_at_token(ident, "inconsistent argument types");

      for (int i = 0; i < arg_types->len; i++)
        if (!same_type(vec_get(arg_types, i), vec_get(func->arg_types, i)))
          error_at_token(ident, "inconsistent argument types");

    } else {
      func->arg_types = arg_types;
    }

    scope = new_scope(scope);
    func_offset = 0;

    for (int i = 0; i < func->arg_types->len; i++) {
      Node *arg = new_node(ND_LVAR, NULL, NULL);
      LVar *lvar = new_var(vec_get(arg_names, i));
      lvar->is_global = false;
      lvar->next = locals;
      lvar->type = vec_get(func->arg_types, i);
      func_offset += lvar->type->size;
      lvar->offset = func_offset;
      locals = lvar;
      arg->lvar = lvar;
      vec_push(func->args, arg);
    }

    node = new_node(ND_FUNC, NULL, NULL);
    Node *block = new_node(ND_BLOCK, NULL, NULL);
    block->scope = scope;
    block->children = new_vec();
    while (!consume("}")) {
      vec_push(block->children, stmt());
    }

    scope = scope->parent;
    node->val = func_offset;

    node->func = func;
    node->body = block;
    return node;
  }

  // Function prototype
  expect(";");
  func->is_proto = true;

  if (func->arg_types->len) {
    if (arg_types->len != func->arg_types->len)
      error_at_token(ident, "inconsistent argument types");

    for (int i = 0; i < arg_types->len; i++)
      if (!same_type(vec_get(arg_types, i), vec_get(func->arg_types, i)))
        error_at_token(ident, "inconsistent argument types");

  } else {
    func->arg_types = arg_types;
  }

  return new_node(ND_DECLARE, NULL, NULL);
}

Node *stmt() {
  // Return statement
  if (consume_token(TK_RETURN)) {
    if (consume(";"))
      return new_node(ND_RETURN, new_node_num(0), NULL);
    Node *node = new_node(ND_RETURN, expr(), NULL);
    expect(";");
    return node;
  }

  // If statement
  if (consume_token(TK_IF)) {
    Node *node = new_node(ND_IF, NULL, NULL);
    node->val = labels++;
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
    node->val = labels++;

    scope = new_scope(scope);
    node->scope = scope;

    br_scope = new_break_scope(br_scope, node);

    expect("(");
    if (!consume(";")) {
      node->init = stmt();
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

    br_scope = br_scope->parent;
    scope = scope->parent;
    return node;
  }

  // While statement
  if (consume_token(TK_WHILE)) {
    Node *node = new_node(ND_WHILE, NULL, NULL);
    br_scope = new_break_scope(br_scope, node);

    node->val = labels++;
    expect("(");
    node->cond = expr();
    expect(")");
    node->body = stmt();

    br_scope = br_scope->parent;
    return node;
  }

  // Do While statement
  if (consume_token(TK_DO)) {
    Node *node = new_node(ND_DO_WHILE, NULL, NULL);
    br_scope = new_break_scope(br_scope, node);

    node->val = labels++;
    node->body = stmt();
    expect_token(TK_WHILE);
    expect("(");
    node->cond = expr();
    expect(")");
    expect(";");

    br_scope = br_scope->parent;
    return node;
  }

  // Switch statement
  if (consume_token(TK_SWITCH)) {
    Node *node = new_node(ND_SWITCH, NULL, NULL);
    node->labels = new_map();

    sw_scope = new_switch_scope(sw_scope, node);
    br_scope = new_break_scope(br_scope, node);

    node->val = labels++;
    expect("(");
    node->cond = expr();
    expect(")");
    node->body = stmt();

    br_scope = br_scope->parent;
    sw_scope = sw_scope->parent;
    return node;
  }

  // Case statement
  if (consume_token(TK_CASE)) {
    Node *node = new_node(ND_CASE, NULL, NULL);
    node->val = labels++;
    node->lhs = expr();
    expect(":");

    if (node->lhs->kind != ND_NUM) error("constant value expected in case statement");
    char *label = calloc(1, sizeof(char) * 10);
    sprintf(label, ".Lcase%03d", node->val);
    map_put(sw_scope->node->labels, label, &(node->lhs->val));

    return node;
  }

  // Default statement
  if (consume_token(TK_DEFAULT)) {
    Node *node = new_node(ND_DEFAULT, NULL, NULL);
    node->val = labels++;
    expect(":");

    char *label = calloc(1, sizeof(char) * 13);
    sprintf(label, ".Ldefault%03d", node->val);
    map_put(sw_scope->node->labels, label, NULL);

    return node;
  }

  // Break statement
  if (consume_token(TK_BREAK)) {
    Node *node = new_node(ND_BREAK, NULL, NULL);
    node->val = br_scope->node->val;
    expect(";");
    return node;
  }

  // Continue statement
  if (consume_token(TK_CONTINUE)) {
    Node *node = new_node(ND_CONTINUE, NULL, NULL);
    node->val = br_scope->node->val;
    expect(";");
    return node;
  }

  // Block
  if (consume("{")) {
    scope = new_scope(scope);
    Node *node = new_node(ND_BLOCK, NULL, NULL);
    node->scope = scope;
    node->children = new_vec();
    while (!consume("}")) {
      vec_push(node->children, stmt());
    }
    scope = scope->parent;
    return node;
  }

  // Define Struct
  Node *node;
  if (node = parse_typedef()) return node;

  // Declare local variable
  Type *ty;
  if (ty = consume_type()) {
    if (ty->ty == STRUCT && consume(";")) return new_node(ND_DECLARE, NULL, NULL);
    if (ty->ty == ENUM && consume(";")) return new_node(ND_DECLARE, NULL, NULL);

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
    if (lvar) error_at_token(ident, "already declared");

    lvar = new_var(ident->str);
    lvar->is_global = false;
    lvar->next = locals;

    if (ty->ty == ARRAY) {
      func_offset += ty->size * ty->array_size;
      lvar->offset = func_offset;
    } else {
      func_offset += ty->size;
      lvar->offset = func_offset;
    }

    lvar->type = ty;
    locals = lvar;

    // Initialize variable
    if (consume("=")) {

      // Initialize with array literal
      if (consume("{")) {
        if (lvar->type->ty != ARRAY) error_at_token(token, "not an array");

        Node *node = new_node(ND_DECLARE, NULL, NULL);
        node->lvar = lvar;

        int index = 0;
        Node *cur = node;

        while (true) {
          Node *nd = new_node(ND_LVAR, NULL, NULL);
          nd->lvar = lvar;
          Node *lhs = new_node(ND_DEREF, new_node(ND_ADD, nd, new_node_num(index++)), NULL);
          Node *child = new_node(ND_ASSIGN, lhs, assign());

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

        func_offset += node->lvar->type->size;
        lvar->offset = func_offset;

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

        str->str = convert_escaped_str(str->str);

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

        func_offset += node->lvar->type->size;
        lvar->offset = func_offset;

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

      Node *node = new_node(ND_ASSIGN, lhs, assign());
      expect(";");
      return node;

    } else if (lvar->type->ty == ARRAY && lvar->type->array_size == 0) {
      error_at_token(token, "array size or initialization is needed");
    }

    Node *node = new_node(ND_DECLARE, NULL, NULL);
    expect(";");
    return node;
  }

  // Expression
  node = expr();
  expect(";");
  return node;
}

Node *expr() {
  Node *node = assign();
  while (true) {
    if (consume(","))
      node = new_node(ND_COMMA, node, assign());
    else
      return node;
  }
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
    nd->val = labels++;
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
    if (consume("||")) {
      node = new_node(ND_LOGICAL_OR, node, logical_and());
      node->val = labels++;
    } else {
      return node;
    }
  }
}

Node *logical_and() {
  Node *node = bitwise_or();
  while (true) {
    if (consume("&&")) {
      node = new_node(ND_LOGICAL_AND, node, bitwise_or());
      node->val = labels++;
    } else {
      return node;
    }
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

  if (consume_token(TK_SIZEOF)) {
    Node *node;
    bool is_paren_open = consume("(");

    Type *ty = consume_type();
    if (ty) {
      LVar *lvar = calloc(1, sizeof(LVar));
      lvar->name = "dummy_for_sizeof";
      lvar->type = ty;

      Node *lvar_node = new_node(ND_LVAR, NULL, NULL);
      lvar_node->lvar = lvar;

      node = new_node(ND_SIZEOF, lvar_node, NULL);

    } else {
      if (is_paren_open)
        node = new_node(ND_SIZEOF, expr(), NULL);
      else
        node = new_node(ND_SIZEOF, unary_left(), NULL);
    }

    if (is_paren_open) expect(")");
    return node;
  }

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
    if (consume("++")) {
      Node *lhs = new_node(ND_ASSIGN, node, new_node(ND_ADD, node, new_node_num(1)));
      Node *rhs = new_node(ND_SUB, node, new_node_num(1));
      node = new_node(ND_COMMA, lhs, rhs);

    } else if (consume("--")) {
      Node *lhs = new_node(ND_ASSIGN, node, new_node(ND_SUB, node, new_node_num(1)));
      Node *rhs = new_node(ND_ADD, node, new_node_num(1));
      node = new_node(ND_COMMA, lhs, rhs);

    } else if (consume("[")) {
      node = new_node(ND_DEREF, new_node(ND_ADD, node, expr()), NULL);
      expect("]");

    } else if (consume("(")) {
      node = new_node(ND_CALL, node, NULL);
      node->children = new_vec();
      if (!consume(")")) {
        while (true) {
          vec_push(node->children, assign());
          if (!consume(",")) break;
        }
        expect(")");
      }

    } else if (consume(".")) {
      Token *ident = consume_ident();
      if (!ident) error_at_token(token, "identificer expected");
      node = new_node(ND_DOT, node, NULL);
      node->name = ident->str;

    } else if (consume("->")) {
      Token *ident = consume_ident();
      if (!ident) error_at_token(token, "identificer expected");
      node = new_node(ND_ARROW, node, NULL);
      node->name = ident->str;

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
    Function *func = map_get(functions, name);
    if (func) {
      node->kind = ND_FUNC_NAME;
      node->func = func;
      return node;
    }

    // Enum constants
    int *enum_const = find_enum_consts(name);
    if (enum_const) {
      node->kind = ND_NUM;
      node->val = *enum_const;
      return node;
    }

    // Local or global variable
    LVar *lvar = find_lvar(ident->str);
    if (!lvar) error_at_token(ident, "not declared");

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
