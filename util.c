#include "9cc.h"

void debug(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
}

void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " ");
  fprintf(stderr, "^ ");

  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

Vector *new_vec() {
  Vector *vec = malloc(sizeof(Vector));
  vec->len = 0;
  vec->capacity = 16;
  vec->data = malloc(sizeof(void *) * 16);
  return vec;
}

void vec_push(Vector *vec, void *elem) {
  if (vec->len == vec->capacity) {
    vec->capacity *= 2;
    vec->data = realloc(vec->data, sizeof(void *) * vec->capacity);
  }
  vec->data[vec->len++] = elem;
}

Map *new_map() {
  Map *map = malloc(sizeof(Map));
  map->keys = new_vec();
  map->vals = new_vec();
  return map;
}

void map_put(Map *map, char *key, void *val) {
  vec_push(map->keys, key);
  vec_push(map->vals, val);
}

void *map_get(Map *map, char *key) {
  for (int i = 0; i < map->keys->len; i++) {
    if (strcmp(map->keys->data[i], key) == 0) {
      return map->vals->data[i];
    }
  }
  return NULL;
}

char *substring(char *str, int len) {
  char *sub = malloc(sizeof(char) * (len + 1));
  strncpy(sub, str, len);
  sub[len] = '\x0';
  return sub;
}

void debug_token(Token *token) {
  Token *tok = token;
  while (tok) {
    switch (tok->kind) {
    case TK_RESERVED:
      fprintf(stderr, "%s ", tok->str);
      break;
    case TK_IDENT:
      fprintf(stderr, "IDENT(%s) ", tok->str);
      break;
    case TK_NUM:
      fprintf(stderr, "NUM(%d) ", tok->val);
      break;
    case TK_RETURN:
      fprintf(stderr, "RETURN ");
      break;
    case TK_IF:
      fprintf(stderr, "IF ");
      break;
    case TK_ELSE:
      fprintf(stderr, "ELSE ");
      break;
    case TK_FOR:
      fprintf(stderr, "FOR ");
      break;
    case TK_WHILE:
      fprintf(stderr, "WHILE ");
      break;
    case TK_EOF:
      fprintf(stderr, "EOF\n");
      break;
    case TK_INT:
      fprintf(stderr, "INT ");
      break;
    case TK_SIZEOF:
      fprintf(stderr, "SIZEOF ");
      break;
    }
    tok = tok->next;
  }
}

void debug_node(Node *node, char *pre1, char *pre2) {
  Node *cur;
  char p11[255], p21[255], p31[255], p41[255], p12[255], p22[255], p32[255], p42[255];
  char *label;
  char *type;

  if (!node) {
    fprintf(stderr, "%sNULL\n", pre1);
    return;
  }

  if (node->type) {
    switch (node->type->ty) {
    case INT:
      type = "INT";
      break;
    case PTR:
      type = "PTR";
      break;
    case ARRAY:
      type = "ARR";
      break;
    }
  } else {
    type = "NUL";
  }

  switch (node->kind) {
  case ND_LVAR:
    fprintf(stderr, "%s%s(%s)\n", pre1, node->lvar->name, type);
    return;

  case ND_GVAR:
    fprintf(stderr, "%s%s(%s)\n", pre1, node->lvar->name, type);
    return;

  case ND_NUM:
    fprintf(stderr, "%s%d(%s)\n", pre1, node->val, type);
    return;

  case ND_DECLARE:
    fprintf(stderr, "%sDECL\n", pre1);
    return;

  case ND_DECLARE_GVAR:
    fprintf(stderr, "%sDECL\n", pre1);
    return;

  case ND_RETURN:
    sprintf(p11, "%sRET ────── ", pre1);
    sprintf(p12, "%s           ", pre2);
    debug_node(node->lhs, p11, p12);
    return;

  case ND_FUNC:
    sprintf(p11, "%sFUNC ───── ", pre1);
    sprintf(p12, "%s           ", pre2);
    debug_node(node->body, p11, p12);
    return;

  case ND_ADDR:
    sprintf(p11, "%sADR(%s) ─ ", pre1, type);
    sprintf(p12, "%s           ", pre2);
    debug_node(node->lhs, p11, p12);
    return;

  case ND_DEREF:
    sprintf(p11, "%sDER(%s) ─ ", pre1, type);
    sprintf(p12, "%s           ", pre2);
    debug_node(node->lhs, p11, p12);
    return;

  case ND_SIZEOF:
    sprintf(p11, "%sSIZEOF ─── ", pre1);
    sprintf(p12, "%s           ", pre2);
    debug_node(node->lhs, p11, p12);
    return;

  case ND_IF:
    sprintf(p11, "%sIF ──────┬ ", pre1);
    sprintf(p12, "%s         │ ", pre2);
    sprintf(p21, "%s         ├ ", pre2);
    sprintf(p22, "%s         │ ", pre2);
    sprintf(p31, "%s         └ ", pre2);
    sprintf(p32, "%s           ", pre2);
    debug_node(node->cond, p11, p12);
    debug_node(node->body, p21, p22);
    debug_node(node->els, p31, p32);
    return;

  case ND_FOR:
    sprintf(p11, "%sFOR ─────┬ ", pre1);
    sprintf(p12, "%s         │ ", pre2);
    sprintf(p21, "%s         ├ ", pre2);
    sprintf(p22, "%s         │ ", pre2);
    sprintf(p31, "%s         ├ ", pre2);
    sprintf(p32, "%s         │ ", pre2);
    sprintf(p41, "%s         └ ", pre2);
    sprintf(p42, "%s           ", pre2);
    debug_node(node->init, p11, p12);
    debug_node(node->cond, p21, p22);
    debug_node(node->inc, p31, p32);
    debug_node(node->body, p41, p42);
    return;

  case ND_WHILE:
    sprintf(p11, "%sWHILE ───┬ ", pre1);
    sprintf(p12, "%s         │ ", pre2);
    sprintf(p21, "%s         └ ", pre2);
    sprintf(p22, "%s           ", pre2);
    debug_node(node->cond, p11, p12);
    debug_node(node->body, p21, p22);
    return;

  case ND_BLOCK:
    cur = node->child;
    while (cur) {
      if (cur == node->child && !cur->next) {
        sprintf(p11, "%sBLOCK ──── ", pre1);
        sprintf(p12, "%s           ", pre2);
      } else if (cur == node->child) {
        sprintf(p11, "%sBLOCK ───┬ ", pre1);
        sprintf(p12, "%s         │ ", pre2);
      } else if (cur->next) {
        sprintf(p11, "%s         ├ ", pre2);
        sprintf(p12, "%s         │ ", pre2);
      } else {
        sprintf(p11, "%s         └ ", pre2);
        sprintf(p12, "%s           ", pre2);
      }
      debug_node(cur, p11, p12);
      cur = cur->next;
    }
    return;

  case ND_CALL:
    cur = node->child;
    while (cur) {
      if (cur == node->child && !cur->next) {
        sprintf(p11, "%sCAL(%s) ─ ", pre1, type);
        sprintf(p12, "%s           ", pre2);
      } else if (cur == node->child) {
        sprintf(p11, "%sCAL(%s) ┬ ", pre1, type);
        sprintf(p12, "%s         │ ", pre2);
      } else if (cur->next) {
        sprintf(p11, "%s         ├ ", pre2);
        sprintf(p12, "%s         │ ", pre2);
      } else {
        sprintf(p11, "%s         └ ", pre2);
        sprintf(p12, "%s           ", pre2);
      }
      debug_node(cur, p11, p12);
      cur = cur->next;
    }
    return;

  case ND_ADD: label = "ADD"; break;
  case ND_SUB: label = "SUB"; break;
  case ND_MUL: label = "MUL"; break;
  case ND_DIV: label = "DIV"; break;
  case ND_ASSIGN: label = "ASS"; break;
  case ND_EQ: label = "EQ "; break;
  case ND_NE: label = "NE "; break;
  case ND_LE: label = "LE "; break;
  case ND_LT: label = "LT "; break;
  }

  sprintf(p11, "%s%s(%s) ┬ ", pre1, label, type);
  sprintf(p12, "%s         │ ", pre2);
  sprintf(p21, "%s         └ ", pre2);
  sprintf(p22, "%s           ", pre2);
  debug_node(node->lhs, p11, p12);
  debug_node(node->rhs, p21, p22);
  return;
}
