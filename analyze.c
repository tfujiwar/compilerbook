#include "9cc.h"

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
}

Type *deref(Type *from) {
  return from->ptr_to;
}

Type *array_to_ptr(Type *array) {
  Type *type = calloc(1, sizeof(Type));
  type->ty = PTR;
  type->size = 8;
  type->ptr_to = array->ptr_to;
  return type;
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

Node *cast_array(Node *from) {
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

  return node;
}

bool is_ptr(Node *node) {
  return node->type->ty == PTR || node->type->ty == ARRAY;
}

Type *bigger_type(Node *n1, Node *n2) {
  if (n1->type->size > n2->type->size)
    return n1->type;
  return n2->type;
}

Node *analyze(Node *node) {
  Node **next;

  switch (node->kind) {
  case ND_ADD:
    node->lhs = analyze(node->lhs);
    node->rhs = analyze(node->rhs);

    if (is_ptr(node->lhs) && is_ptr(node->rhs)) {
      error("invalid operands for add: ptr, ptr");
    }

    if (is_ptr(node->lhs) && !is_ptr(node->rhs)) {
      node->lhs = cast_array(node->lhs);
      return add_ptr_node(node->lhs, node->rhs);
    }

    if (is_ptr(node->rhs) && !is_ptr(node->lhs)) {
      node->rhs = cast_array(node->rhs);
      return add_ptr_node(node->rhs, node->lhs);
    }

    node->type = bigger_type(node->lhs, node->rhs);
    return node;

  case ND_SUB:
    node->lhs = analyze(node->lhs);
    node->rhs = analyze(node->rhs);

    if (!is_ptr(node->lhs) && is_ptr(node->rhs))
      error("invalid operands for sub: num, ptr");

    if (is_ptr(node->lhs) && is_ptr(node->rhs))
      error("not supported operands for sub: ptr, ptr");  // TODO

    if (is_ptr(node->lhs) && !is_ptr(node->rhs))
      return sub_ptr_node(node->lhs, node->rhs);

    node->type = bigger_type(node->lhs, node->rhs);;
    return node;

  case ND_MUL:
  case ND_DIV:
    node->lhs = analyze(node->lhs);
    node->rhs = analyze(node->rhs);

    if (is_ptr(node->lhs) || is_ptr(node->lhs))
      error("invalid operands for mul or div");

    node->type = bigger_type(node->lhs, node->rhs);
    return node;

  case ND_ASSIGN:
    node->lhs = analyze(node->lhs);
    node->rhs = analyze(node->rhs);
    node->rhs = cast_array(node->rhs);

    if (is_ptr(node->lhs) && !same_type(node->lhs->type, node->rhs->type))
      error("invalid type for assign");

    node->type = node->lhs->type;
    return node;

  case ND_LVAR:
  case ND_GVAR:
    node->type = node->lvar->type;
    return node;

  case ND_NUM:
    node->type = type_int();
    return node;

  case ND_EQ:
  case ND_NE:
  case ND_LE:
  case ND_LT:
    node->lhs = analyze(node->lhs);
    node->rhs = analyze(node->rhs);
    node->type = type_int();
    return node;

  case ND_RETURN:
    node->lhs = analyze(node->lhs);
    return node;

  case ND_IF:
    node->cond = analyze(node->cond);
    node->body = analyze(node->body);
    if (node->els) node->els  = analyze(node->els);
    return node;

  case ND_FOR:
    if (node->init) node->init = analyze(node->init);
    if (node->cond) node->cond = analyze(node->cond);
    if (node->inc)  node->inc  = analyze(node->inc);
    node->body = analyze(node->body);
    return node;

  case ND_WHILE:
    node->cond = analyze(node->cond);
    node->body = analyze(node->body);
    return node;

  case ND_BLOCK:
    next = &(node->child);
    while (*next) {
      *next = analyze(*next);
      next = &((*next)->next);
    }
    return node;

  case ND_FUNC:
    node->body = analyze(node->body);
    return node;

  case ND_CALL:
    next = &(node->child);
    while (*next) {
      *next = analyze(*next);
      next = &((*next)->next);
    }
    return node;

  case ND_ADDR:
    node->lhs = analyze(node->lhs);
    node->type = ptr_to(node->lhs->type);
    return node;

  case ND_DEREF:
    node->lhs = analyze(node->lhs);
    node->lhs = cast_array(node->lhs);
    node->type = deref(node->lhs->type);
    return node;

  case ND_SIZEOF:
    node->lhs = analyze(node->lhs);
    if (node->lhs->type->ty == ARRAY)
      return int_node(node->lhs->type->ptr_to->size * node->lhs->type->array_size);
    else
      return int_node(node->lhs->type->size);

  case ND_DECLARE:
    return node;

  case ND_DECLARE_GVAR:
    return node;
  }
}
