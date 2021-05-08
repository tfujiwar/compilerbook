#include "mycc.h"

Type *type_char() {
  Type *type = calloc(1, sizeof(Type));
  type->ty = CHAR;
  type->size = 1;
  return type;
}

Type *type_int() {
  Type *type = calloc(1, sizeof(Type));
  type->ty = INT;
  type->size = 4;
  return type;
}

Type *ptr_to(Type *to) {
  Type *type = calloc(1, sizeof(Type));
  type->ty = PTR;
  type->size = 8;
  type->ptr_to = to;
  return type;
}

bool same_type(Type *t1, Type *t2) {
  if (t1->ty != t2->ty)
    return false;

  if (t1->ty == CHAR || t1->ty == INT)
    return true;

  if (t1->ty == PTR)
    return same_type(t1->ptr_to, t2->ptr_to);

  if (t1->ty == ARRAY)
    return t1->array_size == t2->array_size && same_type(t1->ptr_to, t2->ptr_to);

  if (t1->ty == STRUCT)
    return t1->strct == t2->strct;

  if (t1->ty == ENUM)
    return t1->enm == t2->enm;

  error("unsupported type");
}

Type *deref(Type *from) {
  return from->ptr_to;
}

Node *add_ptr_node(Node *ptr, Node *num) {
  Node *size = calloc(1, sizeof(Node));
  size->kind = ND_NUM;
  size->val = ptr->type->ptr_to->size;
  size->type = type_int();

  Node *mul = calloc(1, sizeof(Node));
  mul->kind = ND_MUL;
  mul->lhs = num;
  mul->rhs = size;
  mul->type = type_int();

  Node *add = calloc(1, sizeof(Node));
  add->kind = ND_ADD;
  add->lhs = ptr;
  add->rhs = mul;
  add->type = ptr->type;

  return add;
}

Node *sub_ptr_node(Node *ptr, Node *num) {
  Node *size = calloc(1, sizeof(Node));
  size->kind = ND_NUM;
  size->val = ptr->type->ptr_to->size;
  size->type = type_int();

  Node *mul = calloc(1, sizeof(Node));
  mul->kind = ND_MUL;
  mul->lhs = num;
  mul->rhs = size;
  mul->type = type_int();

  Node *sub = calloc(1, sizeof(Node));
  sub->kind = ND_SUB;
  sub->lhs = ptr;
  sub->rhs = mul;
  sub->type = ptr->type;

  return sub;
}

Node *int_node(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  node->type = type_int();
  return node;
}

Node *cast_array_to_ptr(Node *from) {
  if (from->type->ty != ARRAY) return from;
  Type *type = calloc(1, sizeof(Type));
  type->ty = PTR;
  type->ptr_to = from->type->ptr_to;
  type->size = 8;
  type->array_size = 0;

  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_ADDR;
  node->lhs = from;
  node->type = type;
  node->next = from->next;

  return node;
}

bool is_ptr_like(Node *node) {
  return node->type->ty == PTR || node->type->ty == ARRAY;
}

Type *bigger_type(Node *n1, Node *n2) {
  if (n1->type->size > n2->type->size)
    return n1->type;
  return n2->type;
}

Node *analyze(Node *node, bool cast_array) {
  Node **next;

  switch (node->kind) {
  case ND_ADD:
    node->lhs = analyze(node->lhs, true);
    node->rhs = analyze(node->rhs, true);

    if (is_ptr_like(node->lhs) && is_ptr_like(node->rhs)) {
      error("invalid operands for add: ptr, ptr");
    }

    if (is_ptr_like(node->lhs) && !is_ptr_like(node->rhs)) {
      return add_ptr_node(node->lhs, node->rhs);
    }

    if (is_ptr_like(node->rhs) && !is_ptr_like(node->lhs)) {
      return add_ptr_node(node->rhs, node->lhs);
    }

    node->type = bigger_type(node->lhs, node->rhs);
    return node;

  case ND_SUB:
    node->lhs = analyze(node->lhs, true);
    node->rhs = analyze(node->rhs, true);

    if (!is_ptr_like(node->lhs) && is_ptr_like(node->rhs))
      error("invalid operands for sub: num, ptr");

    if (is_ptr_like(node->lhs) && is_ptr_like(node->rhs))
      error("not supported operands for sub: ptr, ptr");  // TODO

    if (is_ptr_like(node->lhs) && !is_ptr_like(node->rhs))
      return sub_ptr_node(node->lhs, node->rhs);

    node->type = bigger_type(node->lhs, node->rhs);
    return node;

  case ND_MUL:
  case ND_DIV:
  case ND_MOD:
  case ND_BITWISE_AND:
  case ND_BITWISE_OR:
  case ND_BITWISE_XOR:
  case ND_SHIFT_LEFT:
  case ND_SHIFT_RIGHT:
    node->lhs = analyze(node->lhs, true);
    node->rhs = analyze(node->rhs, true);

    if (is_ptr_like(node->lhs) || is_ptr_like(node->lhs))
      error("invalid operands for mul or div");

    node->type = bigger_type(node->lhs, node->rhs);
    return node;

  case ND_ASSIGN:
    node->lhs = analyze(node->lhs, true);
    node->rhs = analyze(node->rhs, true);
    node->type = node->lhs->type;

    if (node->type->ty == STRUCT) {
      Node *dummy = new_node_num(0);
      Node *comma = new_node(ND_COMMA, dummy, NULL);
      dummy->type = type_int();

      for (int i = 0; i < node->type->strct->member->keys->len; i++) {
        Member *member = node->type->strct->member->vals->data[i];

        Node *offset = new_node_num(member->offset);
        offset->type = type_int();

        Node *lhs_base = new_node(ND_ADDR, node->lhs, NULL);
        Node *rhs_base = new_node(ND_ADDR, node->rhs, NULL);
        lhs_base->type = ptr_to(node->lhs->type);
        rhs_base->type = ptr_to(node->rhs->type);

        Node *lhs_addr = new_node(ND_ADD, lhs_base, offset);
        Node *rhs_addr = new_node(ND_ADD, rhs_base, offset);
        lhs_addr->type = ptr_to(member->type);
        rhs_addr->type = ptr_to(member->type);

        Node *lhs_deref = new_node(ND_DEREF, lhs_addr, NULL);
        Node *rhs_deref = new_node(ND_DEREF, rhs_addr, NULL);
        lhs_deref->type = member->type;
        rhs_deref->type = member->type;

        comma->rhs = new_node(ND_ASSIGN, lhs_deref, rhs_deref);
        comma->rhs->type = member->type;

        Node *next_comma = new_node(ND_COMMA, NULL, NULL);
        next_comma->lhs = comma;
        comma = next_comma;
      }
      comma->rhs = node->lhs;
      comma->type = node->lhs->type;
      return comma;

    }
    return node;

  case ND_LVAR:
  case ND_GVAR:
    node->type = node->lvar->type;
    if (cast_array) node = cast_array_to_ptr(node);
    return node;

  case ND_NUM:
    node->type = type_int();
    return node;

  case ND_EQ:
  case ND_NE:
  case ND_LE:
  case ND_LT:
  case ND_LOGICAL_AND:
  case ND_LOGICAL_OR:
    node->lhs = analyze(node->lhs, true);
    node->rhs = analyze(node->rhs, true);
    node->type = type_int();
    return node;

  case ND_CONDITIONAL:
    node->cond = analyze(node->cond, true);
    node->body = analyze(node->body, true);
    node->els = analyze(node->els, true);
    node->type = type_int();
    return node;

  case ND_RETURN:
    node->lhs = analyze(node->lhs, true);
    return node;

  case ND_IF:
    node->cond = analyze(node->cond, true);
    node->body = analyze(node->body, true);
    if (node->els) node->els  = analyze(node->els, true);
    return node;

  case ND_FOR:
    scope = node->scope;
    if (node->init) node->init = analyze(node->init, true);
    if (node->cond) node->cond = analyze(node->cond, true);
    if (node->inc)  node->inc  = analyze(node->inc, true);
    node->body = analyze(node->body, true);
    scope = scope->parent;
    return node;

  case ND_WHILE:
  case ND_DO_WHILE:
    node->cond = analyze(node->cond, true);
    node->body = analyze(node->body, true);
    return node;

  case ND_SWITCH:
    node->cond = analyze(node->cond, true);
    node->body = analyze(node->body, true);
    return node;

  case ND_CASE:
  case ND_DEFAULT:
  case ND_BREAK:
  case ND_CONTINUE:
    return node;

  case ND_BLOCK:
    scope = node->scope;
    for (int i = 0; i < node->children->len; i++) {
      node->children->data[i] = analyze(node->children->data[i], true);
    }
    scope = scope->parent;
    return node;

  case ND_FUNC:
    node->body = analyze(node->body, true);

    for (int i = 0; i < node->func->args->len; i++) {
      Node *nd = vec_get(node->func->args, i);
      nd->type = nd->lvar->type;
    }

    return node;

  case ND_CALL:
    node->lhs = analyze(node->lhs, true);
    node->type = node->lhs->type;

    for (int i = 0; i < node->children->len; i++) {
      node->children->data[i] = analyze(node->children->data[i], true);
    }

    return node;

  case ND_FUNC_NAME:
    node->type = node->func->return_type;
    return node;

  case ND_ADDR:
    node->lhs = analyze(node->lhs, false);
    node->type = ptr_to(node->lhs->type);
    return node;

  case ND_DEREF:
    node->lhs = analyze(node->lhs, true);
    node->type = deref(node->lhs->type);
    return node;

  case ND_SIZEOF:
    node->lhs = analyze(node->lhs, false);
    return int_node(node->lhs->type->size);

  case ND_BITWISE_NOT:
    node->lhs = analyze(node->lhs, true);
    node->type = type_int();
    return node;

  case ND_DECLARE:
    next = &(node->child);
    while (*next) {
      *next = analyze(*next, true);
      next = &((*next)->next);
    }
    return node;

  case ND_DECLARE_GVAR:
    node->type = node->lvar->type;
    next = &(node->rhs);
    while (*next) {
      *next = analyze(*next, true);
      next = &((*next)->next);
    }
    return node;

  case ND_STRING:
    node->type = ptr_to(type_char());
    return node;

  case ND_COMMA:
    node->lhs = analyze(node->lhs, true);
    node->rhs = analyze(node->rhs, true);
    node->type = node->rhs->type;
    return node;

  case ND_DOT:
    node->lhs = analyze(node->lhs, true);
    if (node->lhs->type->ty != STRUCT) error("struct expected: %d", node->lhs->type->ty);

    Type *type = find_struct(node->lhs->type->strct->name);
    if (!type) error("type not found");

    Member *member = map_get(type->strct->member, node->name);

    Node *base = new_node(ND_ADDR, node->lhs, NULL);
    base->type = ptr_to(type);

    Node *offset = new_node_num(member->offset);
    offset->type = type_int();

    Node *addr = new_node(ND_ADD, base, offset);
    addr->type = ptr_to(member->type);

    Node *nd = new_node(ND_DEREF, addr, NULL);
    nd->type = member->type;

    return nd;

  case ND_ARROW:
    node->lhs = analyze(node->lhs, true);
    if (node->lhs->type->ty != PTR || node->lhs->type->ptr_to->ty != STRUCT)
      error("struct expected: %d", node->lhs->type->ty);

    type = find_struct(node->lhs->type->ptr_to->strct->name);
    if (!type) error("type not found");

    member = map_get(type->strct->member, node->name);

    base = node->lhs;

    offset = new_node_num(member->offset);
    offset->type = type_int();

    addr = new_node(ND_ADD, base, offset);
    addr->type = ptr_to(member->type);

    nd = new_node(ND_DEREF, addr, NULL);
    nd->type = member->type;

    return nd;

  default:
    error("failed to analyze node: %d", node->kind);
  }
}
