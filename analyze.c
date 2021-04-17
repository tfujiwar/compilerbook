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

  if (t1->ty == INT)
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
  size->val = ptr->type->size;
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
  size->val = ptr->type->size;
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

Node *cast_array(Node *node) {
  if (node->type->ty != ARRAY) return node;
  Type *type = calloc(1, sizeof(Type));
  type->ty = PTR;
  type->ptr_to = node->type->ptr_to;
  type->size = 8;
  type->array_size = 0;
  node->type = type;
  return node;
}

Node *analyze(Node *node) {
  Node **next;

  switch (node->kind) {
  case ND_ADD:
    node->lhs = analyze(node->lhs);
    node->rhs = analyze(node->rhs);

    if ((node->lhs->type->ty == PTR || node->lhs->type->ty == ARRAY) && node->rhs->type->ty == INT) {
      node->lhs = cast_array(node->lhs);
      return add_ptr_node(node->lhs, node->rhs);
    }

    if ((node->rhs->type->ty == PTR || node->rhs->type->ty == ARRAY) && node->lhs->type->ty == INT) {
      node->rhs = cast_array(node->rhs);
      return add_ptr_node(node->rhs, node->lhs);
    }

    if (same_type(node->lhs->type, node->rhs->type)) {
      node->type = node->lhs->type;
      return node;
    }

    error("invalid type for add");
    break;

  case ND_SUB:
    node->lhs = analyze(node->lhs);
    node->rhs = analyze(node->rhs);

    if (node->lhs->type->ty == PTR && node->rhs->type->ty == INT)
      return sub_ptr_node(node->lhs, node->rhs);

    if (same_type(node->lhs->type, node->rhs->type)) {
      node->type = node->lhs->type;
      return node;
    }

    error("invalid type for sub");
    break;

  case ND_MUL:
  case ND_DIV:
    node->lhs = analyze(node->lhs);
    node->rhs = analyze(node->rhs);

    if (!same_type(node->lhs->type, node->rhs->type))
      error("invalid type for mul or div");

    if (node->lhs->type->ty != INT)
      error("invalid type for mul or div");

    node->type = type_int();
    return node;

  case ND_ASSIGN:
    node->lhs = analyze(node->lhs);
    node->rhs = analyze(node->rhs);
    node->rhs = cast_array(node->rhs);

    if (!same_type(node->lhs->type, node->rhs->type))
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

    if (!same_type(node->lhs->type, node->rhs->type))
      error("invalid type for comparison");

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
    return node;  // TODO

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
